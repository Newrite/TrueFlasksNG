const flasks = [
    { id: 'flask-health', type: 0, color: '#6a0020' },
    { id: 'flask-stamina', type: 1, color: '#2f7d33' },
    { id: 'flask-magick', type: 2, color: '#2f4ba4' },
    { id: 'flask-other', type: 3, color: '#7b4997' }
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
                    transition: transform 0.3s ease-out;
                    will-change: transform; /* Подсказка рендеру */
                }

                .flask-counter {
                    font-family: 'Impact', 'Bahnschrift', sans-serif;
                    font-weight: 400;
                    font-size: 90px;

                    /* === ОПТИМИЗАЦИЯ ПОД CPU === */
                    fill: white;           /* Цвет заливки */
                    stroke: black;         /* Цвет обводки (нативный SVG) */
                    stroke-width: 5px;     /* Толщина обводки */
                    stroke-linejoin: round; /* Скругленные углы обводки */

                    /* Ключевое свойство: рисует обводку ПОЗАДИ заливки */
                    /* Поддерживается в WebKit 615+ */
                    paint-order: stroke fill;

                    letter-spacing: 2px;
                    pointer-events: none;
                    user-select: none;

                    /* Убираем text-shadow, так как stroke делает работу быстрее и чище */
                }
            `;
            svgDoc.documentElement.appendChild(styleElement);
        }

        // 3. Создаем текстовый элемент
        let textElement = svgDoc.querySelector('.flask-counter');
        if (!textElement) {
            textElement = svgDoc.createElementNS("http://www.w3.org/2000/svg", "text");
            textElement.textContent = "";
            textElement.setAttribute("class", "flask-counter");
            // Координаты центрирования (подбираются под SVG)
            textElement.setAttribute("x", "242");
            textElement.setAttribute("y", "266");
            textElement.setAttribute("dominant-baseline", "central");
            textElement.setAttribute("text-anchor", "middle");
            svgRoot.appendChild(textElement);
        }

        // Сохраняем ссылки
        flaskElements[item.type] = {
            object: obj,
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

            // Сброс индивидуальных стилей
            for (const key in flaskElements) {
                const el = flaskElements[key].object;
                el.style.transform = '';
                el.style.opacity = '';
                el.style.left = '';
                el.style.top = '';
            }

            flasks.forEach(f => {
                const obj = document.getElementById(f.id);
                if (obj) obj.style.opacity = '1';
            });

        } else {
            container.style.left = '0px';
            container.style.top = '0px';
            container.style.transform = 'scale(1)';
            container.style.opacity = '1';

            const applyFlaskSettings = (type, s) => {
                const item = flasks.find(f => f.type === type);
                if (!item) return;
                const el = document.getElementById(item.id);

                if (el) {
                    el.style.left = (s.x * window.innerWidth) + 'px';
                    el.style.top = (s.y * window.innerHeight) + 'px';
                    el.style.transform = `scale(${s.size})`;
                    el.style.opacity = s.opacity;
                }
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

    // Обновляем уровень жидкости
    if (el.fillRect) {
        el.fillRect.style.transform = `scaleY(${fillPercent})`;
    }

    // Обновляем текст
    if (el.text) {
        // Показываем цифру только если она больше 0
        el.text.textContent = count > 0 ? count : "";
    }

    // Логика свечения
    const isFull = fillPercent >= 0.99;
    let triggerGlow = shouldGlow;

    if (isFull && !el.wasFull) {
        triggerGlow = true;
    }
    el.wasFull = isFull;

    if (triggerGlow) {
        // Важно: для CPU рендеринга лучше не использовать box-shadow анимации.
        // Ограничиваемся opacity или transform анимациями, если они прописаны в CSS самого элемента.
        el.object.style.animationPlayState = 'running';
        setTimeout(() => {
            el.object.style.animationPlayState = 'paused';
        }, 3000);
    }

    if (el.object.style.opacity === '' || el.object.style.opacity === '0') {
        el.object.style.opacity = '1';
    }
};

document.addEventListener('DOMContentLoaded', () => {
    setTimeout(() => window.firstInitDom(), 1000);
});
