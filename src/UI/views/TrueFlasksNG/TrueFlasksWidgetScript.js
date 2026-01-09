const flasks = [
    { id: 'flask-health', counterId: 'flask-counter-health', type: 0, color: '#6a0020' },
    { id: 'flask-stamina', counterId: 'flask-counter-stamina', type: 1, color: '#2f7d33' },
    { id: 'flask-magick', counterId: 'flask-counter-magick', type: 2, color: '#2f4ba4' },
    { id: 'flask-other', counterId: 'flask-counter-other', type: 3, color: '#7b4997' }
];

// Кэш элементов
const flaskElements = {};

function initFlask(item) {
    const obj = document.getElementById(item.id);
    if (!obj) return;

    // Устанавливаем цвет свечения (используется в CSS анимациях, если они есть)
    obj.style.setProperty('--glow-color', item.color);

    const setupSvg = () => {
        if (flaskElements[item.type]) return;

        const svgDoc = obj.contentDocument;
        if (!svgDoc) {
            console.warn(`Cannot access content of ${item.id}.`);
            return;
        }

        const svgRoot = svgDoc.documentElement;

        // 1. Расширяем viewBox, чтобы текст и обводка не обрезались
        if (!svgRoot.hasAttribute('data-expanded')) {
            const originalVB = svgRoot.getAttribute('viewBox').split(' ').map(parseFloat);
            const padding = 80; // Запас места
            const newX = originalVB[0] - padding;
            const newY = originalVB[1] - padding;
            const newW = originalVB[2] + (padding * 2);
            const newH = originalVB[3] + (padding * 2);
            svgRoot.setAttribute('viewBox', `${newX} ${newY} ${newW} ${newH}`);
            svgRoot.setAttribute('data-expanded', 'true');
        }

        // 2. Внедряем стили (ОПТИМИЗИРОВАНО ДЛЯ CPU)
        if (!svgDoc.getElementById('flask-styles')) {
            const styleElement = svgDoc.createElementNS("http://www.w3.org/2000/svg", "style");
            styleElement.id = 'flask-styles';
            styleElement.textContent = `
                #flask-fill-rect {
                    transform-origin: bottom;
                    transform-box: fill-box;
                    transform: scaleY(0);
                    /* Используем transform для анимации уровня — это дешевле для CPU */
                    /* Fast transition for responsiveness */
                    transition: transform 0.1s linear;
                    will-change: transform; /* Подсказка рендеру */
                }
            `;
            svgDoc.documentElement.appendChild(styleElement);
        }

        // 3. Ссылка на HTML текстовый элемент
        const textElement = document.getElementById(item.counterId);

        // Сохраняем ссылки
        // object: сам object tag (для glow animation)
        // wrapper: родительский div (для позиционирования)
        flaskElements[item.type] = {
            object: obj,
            wrapper: obj.parentElement,
            svgRoot: svgRoot,
            fillRect: svgDoc.getElementById('flask-fill-rect'),
            text: textElement,
            wasFull: false
        };
    };

    if (obj.contentDocument && obj.contentDocument.documentElement) {
        setupSvg();
    } else {
        obj.addEventListener('load', setupSvg);
    }
}

window.firstInitDom = () => {
    flasks.forEach(initFlask);
}

// === Настройки виджета ===
window.setWidgetSettings = (settingsJson) => {
    try {
        const settings = JSON.parse(settingsJson);
        const container = document.getElementById('widget-container');

        if (!container) return;

        if (!settings.enable) {
            container.style.display = 'none';
            return;
        }
        container.style.display = 'block';

        if (settings.anchor_all) {
            container.style.left = (settings.x * window.innerWidth) + 'px';
            container.style.top = (settings.y * window.innerHeight) + 'px';
            container.style.transform = `scale(${settings.size})`;
            container.style.opacity = settings.opacity;

            // Сброс индивидуальных стилей и показ элементов (если они включены)
            flasks.forEach(item => {
                let wrapper;
                if (flaskElements[item.type] && flaskElements[item.type].wrapper) {
                    wrapper = flaskElements[item.type].wrapper;
                } else {
                    const obj = document.getElementById(item.id);
                    if (obj) wrapper = obj.parentElement;
                }

                if (wrapper) {
                    // Получаем настройки для конкретной фласки
                    const flaskSettings = 
                        (item.type === 0) ? settings.health :
                        (item.type === 1) ? settings.stamina :
                        (item.type === 2) ? settings.magick :
                        settings.other;

                    wrapper.style.transform = '';
                    wrapper.style.left = '';
                    wrapper.style.top = '';
                    
                    // Если фласка выключена в конфиге, скрываем её
                    if (flaskSettings && flaskSettings.enabled === false) {
                        wrapper.style.opacity = '0';
                    } else {
                        wrapper.style.opacity = '1';
                    }
                }
            });

        } else {
            container.style.left = '0px';
            container.style.top = '0px';
            container.style.transform = 'scale(1)';
            container.style.opacity = '1';

            const applyFlaskSettings = (type, s) => {
                let el;
                // Try cache
                if (flaskElements[type] && flaskElements[type].wrapper) {
                    el = flaskElements[type].wrapper;
                } else {
                    // Fallback to DOM
                    const item = flasks.find(f => f.type === type);
                    if (item) {
                        const obj = document.getElementById(item.id);
                        if (obj) el = obj.parentElement;
                    }
                }
                
                if (!el) return;

                if (s.enabled === false) {
                    el.style.opacity = '0';
                    return;
                }

                // If settings object is missing or disabled logic is needed
                // For now apply what we have
                el.style.left = (s.x * window.innerWidth) + 'px';
                el.style.top = (s.y * window.innerHeight) + 'px';
                el.style.transform = `scale(${s.size})`;
                el.style.opacity = s.opacity;
            };

            applyFlaskSettings(0, settings.health);
            applyFlaskSettings(1, settings.stamina);
            applyFlaskSettings(2, settings.magick);
            applyFlaskSettings(3, settings.other);
        }

    } catch (e) {
        console.error("Error parsing settings JSON", e);
    }
};

window.setWidgetSettingsInit = (settingsJson) => {
    setTimeout(() => window.setWidgetSettings(settingsJson), 1500);
}

// === Обновление данных (Поддержка JSON) ===
window.updateFlaskData = (args) => {
    if (!args) return;

    let flaskType, fillPercent, count, shouldGlow;

    // Проверяем, пришел ли JSON (начинается с {)
    if (args.trim().startsWith('{')) {
        try {
            const params = JSON.parse(args);
            flaskType = parseInt(params.typeIndex);
            fillPercent = parseFloat(params.percent);
            count = parseInt(params.count);
            // Преобразуем строку "true" в булево
            shouldGlow = (String(params.forceGlow).toLowerCase() === 'true');
        } catch(e) {
            console.error("JSON parse error", e);
            return;
        }
    } else {
        // Старый формат CSV: "0,0.5,5,1"
        const parts = args.split(',');
        if (parts.length < 4) return;
        flaskType = parseInt(parts[0]);
        fillPercent = parseFloat(parts[1]);
        count = parseInt(parts[2]);
        shouldGlow = parts[3] === '1';
    }

    const el = flaskElements[flaskType];
    if (!el) return;

    // Fix Visual Sync: Clamp visual fill
    // Logic: 
    // If count > 0, flask is visually full (1.0).
    // If count == 0, flask shows cooldown progress (fillPercent).
    // But we want to avoid it looking full when it's almost done cooling down.
    
    let visualPercent = fillPercent;
    
    // Fix Visual Range: Map 0.0-1.0 to the visible liquid area
    // The fillRect covers the whole canvas (484px), but liquid is only in the middle.
    // Approx range: 15% to 90%
    const minVis = 0.15;
    const maxVis = 0.90;
    
    let mappedPercent = 0;
    if (visualPercent > 0.01) {
         mappedPercent = minVis + (visualPercent * (maxVis - minVis));
    }
    // If fully full (1.0), ensure it covers everything just in case
    if (visualPercent >= 1.0) mappedPercent = 1.0;


    // Обновляем уровень жидкости
    if (el.fillRect) {
        el.fillRect.style.transform = `scaleY(${mappedPercent})`;
    }

    // Обновляем текст (HTML элемент)
    if (el.text) {
        // Показываем цифру только если она больше 0
        el.text.textContent = count > 0 ? count : "";
    }

    // Логика свечения (анимация на объекте svg)
    // Glow if full (count > 0) and wasn't before? Or if cooldown finished?
    // "Если банка не была заполнена и заполнилась" -> transition from count=0 to count=1
    // Or transition from fillPercent < 1.0 to fillPercent >= 1.0 (which usually coincides with count increment)
    
    // Let's track "isReady" state. Ready means count > 0.
    const isReady = count > 0;
    let triggerGlow = shouldGlow;

    if (isReady && !el.wasFull) {
        triggerGlow = true;
    }
    el.wasFull = isReady;

    if (triggerGlow) {
        el.object.classList.remove('glowing');
        void el.object.offsetWidth; // Trigger reflow
        el.object.classList.add('glowing');
    }

    // Убедимся, что враппер видим
    if (el.wrapper && (el.wrapper.style.opacity === '' || el.wrapper.style.opacity === '0')) {
        el.wrapper.style.opacity = '1';
    }
};

document.addEventListener('DOMContentLoaded', () => {
    setTimeout(() => window.firstInitDom(), 1000);
});
