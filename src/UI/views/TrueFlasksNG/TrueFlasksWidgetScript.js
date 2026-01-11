// =========================================================
// НАСТРОЙКИ ВИЗУАЛИЗАЦИИ
// =========================================================

// 0.0 - самый низ картинки SVG
// 1.0 - самый верх картинки SVG

// Уровень дна (откуда начинается жидкость)
const VISUAL_BOTTOM = 0.20;

// Уровень горлышка (где жидкость должна остановиться при 100%)
// ВАЖНО: Именно этим параметром мы лечим проблему "анимация закончилась, а цифра не сменилась".
// Если при 100% (заряд восстановился) жидкость "улетает" выше горлышка — уменьшайте это число.
// Если жидкость не доходит — увеличивайте.
const VISUAL_TOP = 0.65;

// =========================================================

const flasks = [
    { id: 'flask-health', counterId: 'flask-counter-health', type: 0, color: '#6a0020' },
    { id: 'flask-stamina', counterId: 'flask-counter-stamina', type: 1, color: '#2f7d33' },
    { id: 'flask-magick', counterId: 'flask-counter-magick', type: 2, color: '#2f4ba4' },
    { id: 'flask-other', counterId: 'flask-counter-other', type: 3, color: '#7b4997' }
];

const flaskElements = {};
let globalSettings = { auto_hide: false };

function initFlask(item) {
    const obj = document.getElementById(item.id);
    if (!obj) return;

    obj.style.setProperty('--glow-color', item.color);

    const setupSvg = () => {
        if (flaskElements[item.type]) return;

        const svgDoc = obj.contentDocument;
        if (!svgDoc) return;

        const svgRoot = svgDoc.documentElement;

        // Расширяем viewBox для эффектов
        if (!svgRoot.hasAttribute('data-expanded')) {
            const originalVB = svgRoot.getAttribute('viewBox').split(' ').map(parseFloat);
            const padding = 80;
            svgRoot.setAttribute('viewBox', `${originalVB[0] - padding} ${originalVB[1] - padding} ${originalVB[2] + (padding * 2)} ${originalVB[3] + (padding * 2)}`);
            svgRoot.setAttribute('data-expanded', 'true');
        }

        // Стиль без transition (мгновенная реакция на проценты)
        if (!svgDoc.getElementById('flask-styles')) {
            const styleElement = svgDoc.createElementNS("http://www.w3.org/2000/svg", "style");
            styleElement.id = 'flask-styles';
            styleElement.textContent = `
                #flask-fill-rect {
                    transform-origin: bottom;
                    transform-box: fill-box;
                    transform: scaleY(0); 
                    will-change: transform;
                }
            `;
            svgDoc.documentElement.appendChild(styleElement);
        }

        const textElement = document.getElementById(item.counterId);

        flaskElements[item.type] = {
            object: obj,
            wrapper: obj.parentElement,
            svgRoot: svgRoot,
            fillRect: svgDoc.getElementById('flask-fill-rect'),
            text: textElement,
            lastCount: -1, // Для отслеживания изменений количества
            maxSlots: -1
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

// === Настройки позиционирования (без изменений логики) ===
window.setWidgetSettings = (settingsJson) => {
    try {
        const settings = JSON.parse(settingsJson);
        globalSettings = settings;
        const container = document.getElementById('widget-container');
        if (!container) return;

        container.style.display = settings.enable ? 'block' : 'none';
        if (!settings.enable) return;

        if (settings.anchor_all) {
            container.style.left = (settings.x * window.innerWidth) + 'px';
            container.style.top = (settings.y * window.innerHeight) + 'px';
            container.style.transform = `scale(${settings.size})`;
            container.style.opacity = settings.opacity;

            flasks.forEach(item => {
                let wrapper = flaskElements[item.type]?.wrapper || document.getElementById(item.id)?.parentElement;
                if (wrapper) {
                    const flaskSettings = (item.type === 0) ? settings.health : (item.type === 1) ? settings.stamina : (item.type === 2) ? settings.magick : settings.other;
                    wrapper.style.transform = ''; wrapper.style.left = ''; wrapper.style.top = '';
                    // If auto_hide is enabled, opacity is controlled by updateFlaskData
                    if (!settings.auto_hide) {
                        wrapper.style.opacity = (flaskSettings && flaskSettings.enabled === false) ? '0' : '1';
                    } else {
                         // Initial state for auto-hide: hidden if we don't know status, or visible if we assume full?
                         // Let's keep it hidden until first update
                         if (flaskElements[item.type] && flaskElements[item.type].maxSlots === -1) {
                             wrapper.style.opacity = '0';
                         }
                    }
                }
            });
        } else {
            container.style.left = '0px'; container.style.top = '0px'; container.style.transform = 'scale(1)'; container.style.opacity = '1';
            const apply = (type, s) => {
                let el = flaskElements[type]?.wrapper || document.getElementById(flasks.find(f=>f.type===type)?.id)?.parentElement;
                if (!el) return;
                if (s.enabled === false) { el.style.opacity = '0'; return; }
                el.style.left = (s.x * window.innerWidth) + 'px'; el.style.top = (s.y * window.innerHeight) + 'px';
                el.style.transform = `scale(${s.size})`; 
                if (!settings.auto_hide) {
                    el.style.opacity = s.opacity;
                }
            };
            apply(0, settings.health); apply(1, settings.stamina); apply(2, settings.magick); apply(3, settings.other);
        }
    } catch (e) { console.error(e); }
};

window.setWidgetSettingsInit = (settingsJson) => setTimeout(() => window.setWidgetSettings(settingsJson), 1500);


// === ГЛАВНАЯ ЛОГИКА ОБНОВЛЕНИЯ ===
window.updateFlaskData = (args) => {
    if (!args) return;

    let flaskType, fillPercent, count, maxSlots, shouldGlow;

    // Парсинг
    if (args.trim().startsWith('{')) {
        try {
            const params = JSON.parse(args);
            flaskType = parseInt(params.typeIndex);
            fillPercent = parseFloat(params.percent); // Чистое значение от 0.0 до 1.0
            count = parseInt(params.count);
            maxSlots = parseInt(params.max_slots);
            shouldGlow = params.forceGlow;
        } catch(e) { return; }
    } else {
        // Legacy support just in case, though C++ sends JSON now
        const parts = args.split(',');
        if (parts.length < 4) return;
        flaskType = parseInt(parts[0]);
        fillPercent = parseFloat(parts[1]);
        count = parseInt(parts[2]);
        shouldGlow = parts[3] === '1';
        maxSlots = 1; // Default fallback
    }

    const el = flaskElements[flaskType];
    if (!el) return;

    el.maxSlots = maxSlots;

    // ---------------------------------------------------------
    // 1. Визуализация заполнения (Mapping)
    // ---------------------------------------------------------

    // Мы полностью доверяем fillPercent.
    // Если fillPercent = 0.5 (пол кулдауна), мы заполняем половину ВИДИМОЙ области.
    // Формула: Нижняя граница + (Процент * (Верхняя граница - Нижняя граница))

    const visualRange = VISUAL_TOP - VISUAL_BOTTOM;
    const mappedScale = VISUAL_BOTTOM + (fillPercent * visualRange);

    // Применяем
    if (el.fillRect) {
        el.fillRect.style.transform = `scaleY(${mappedScale})`;
    }

    // ---------------------------------------------------------
    // 2. Текст
    // ---------------------------------------------------------
    if (el.text) {
        el.text.textContent = count > 0 ? count : "";
    }

    // ---------------------------------------------------------
    // 3. Свечение
    // ---------------------------------------------------------

    let triggerGlow = shouldGlow;

    // Инициализация lastCount, чтобы не светилось при первом запуске
    if (el.lastCount === -1) {
        el.lastCount = count;
    }

    // Если количество увеличилось (0 -> 1, 1 -> 2 и т.д.), запускаем свечение
    if (count > el.lastCount) {
        triggerGlow = true;
    }
    el.lastCount = count;

    if (triggerGlow) {
        el.object.classList.remove('glowing');
        void el.object.offsetWidth; // Перезапуск анимации CSS
        el.object.classList.add('glowing');
    }

    // ---------------------------------------------------------
    // 4. Автоскрытие / Показ элемента
    // ---------------------------------------------------------
    if (el.wrapper) {
        let targetOpacity = '1';
        
        // Если включено автоскрытие
        if (globalSettings.auto_hide) {
            // Скрываем, если количество зарядов равно максимальному (полная фласка)
            if (count >= maxSlots) {
                targetOpacity = '0';
            } else {
                targetOpacity = '1';
            }
        } else {
             // Если автоскрытие выключено, берем прозрачность из настроек (если anchor_all) или индивидуальную
             // Но здесь мы просто ставим 1, так как прозрачность уже задана в setWidgetSettings
             // Однако, если элемент был скрыт (opacity 0) из-за инициализации, надо показать
             if (el.wrapper.style.opacity === '0' && !globalSettings.auto_hide) {
                 // Check if it was disabled in settings
                 // This logic is a bit complex because we don't have easy access to individual settings here without reparsing
                 // Assuming setWidgetSettings handled the base opacity correctly.
                 // If we are here, updateFlaskData is called, implying the widget is active/updating.
                 // Let's just ensure it's visible if it was hidden by auto-hide logic previously?
                 // Actually, setWidgetSettings sets opacity based on config.
                 // If we override it here to '1', we might break custom opacity.
                 // Let's only touch opacity if auto_hide is ON.
                 // BUT: The user requirement says "Works if auto-hide functionality is enabled".
                 // So if auto_hide is OFF, we do nothing here regarding opacity.
             }
        }

        if (globalSettings.auto_hide) {
             // Apply fade
             el.wrapper.style.transition = 'opacity 0.5s ease';
             el.wrapper.style.opacity = targetOpacity;
        }
    }
};

document.addEventListener('DOMContentLoaded', () => {
    setTimeout(() => window.firstInitDom(), 1000);
});

// =========================================================
// DEBUG / BROWSER TESTING MODE
// =========================================================

// Запускаем только когда все картинки и стили полностью загрузились

const DEBUG_FLASKS = false;

window.addEventListener('load', function() {
    
    if (!DEBUG_FLASKS) {
        return;
    }

    // Проверка: мы в браузере или в игре?
    const isBrowser = (typeof window !== 'undefined' && (window.location.protocol === 'file:' || window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1'));

    // Если нужно принудительно включить тест, раскомментируйте строку ниже:
    // const FORCE_TEST = true; 

    if (isBrowser || (typeof FORCE_TEST !== 'undefined' && FORCE_TEST)) {
        console.group("%c TrueFlasks Debug Started ", "background: #222; color: #bada55; font-size: 14px");
        console.log("Environment detected: Browser/Debug");

        // 1. Настраиваем фон
        document.body.style.backgroundColor = "#1a1a1a";
        document.body.style.backgroundImage = "linear-gradient(45deg, #1a1a1a 25%, #2a2a2a 25%, #2a2a2a 50%, #1a1a1a 50%, #1a1a1a 75%, #2a2a2a 75%, #2a2a2a 100%)";
        document.body.style.backgroundSize = "20px 20px";

        // 2. Проверяем доступ к SVG (Самая частая проблема)
        setTimeout(() => {
            const testObj = document.getElementById('flask-health');
            if (testObj) {
                try {
                    const doc = testObj.contentDocument;
                    if (!doc) {
                        console.error("❌ ОШИБКА ДОСТУПА К SVG: obj.contentDocument is null.");
                        console.warn("💡 РЕШЕНИЕ: Не открывайте файл напрямую через проводник (file://). Используйте локальный сервер (Live Server в VS Code или python http.server).");
                        alert("Ошибка: Браузер заблокировал доступ к SVG файлам.\nСкрипт не может управлять заливкой.\n\nЗапустите через локальный сервер (Live Server)!");
                        return;
                    } else {
                        console.log("✅ Доступ к SVG есть. Начинаем симуляцию.");
                    }
                } catch (e) {
                    console.error("Ошибка безопасности:", e);
                }
            } else {
                console.error("❌ Не найден элемент с id='flask-health'. Проверьте HTML.");
            }

            // 3. Инициализация настроек
            const mockSettings = {
                enable: true,
                x: 0, y: 0, size: 1.0, opacity: 1.0,
                anchor_all: false,
                auto_hide: true, // Test auto hide
                health:  { enabled: true, x: 0.3, y: 0.4, size: 0.8, opacity: 1.0 },
                stamina: { enabled: true, x: 0.4, y: 0.4, size: 0.8, opacity: 1.0 },
                magick:  { enabled: true, x: 0.5, y: 0.4, size: 0.8, opacity: 1.0 },
                other:   { enabled: true, x: 0.6, y: 0.4, size: 0.8, opacity: 1.0 }
            };

            // Вызываем настройку
            if (window.setWidgetSettings) {
                window.setWidgetSettings(JSON.stringify(mockSettings));
                // Принудительно инициализируем DOM, если еще не
                window.firstInitDom();
            }

            // 4. Цикл анимации
            let time = 0;
            let healthVal = 0.0;
            let magicVal = 1.0;
            let magicCount = 2;
            let magicCooldown = false;

            setInterval(() => {
                time += 16;

                // --- HEALTH (Красная): Медленно заполняется ---
                healthVal += 0.005;
                if (healthVal > 1.3) healthVal = 0.0; // Сброс

                // ВАЖНО: Тестируем логику count
                // Пока меньше 1.0 - count 0. Как стало 1.0 - count 1.
                let hPercent = Math.min(healthVal, 1.0);
                let hCount = healthVal >= 1.0 ? 1 : 0;

                window.updateFlaskData(JSON.stringify({
                    typeIndex: 0,
                    percent: hPercent,
                    count: hCount,
                    max_slots: 1,
                    forceGlow: false
                }));

                // --- MAGICK (Синяя): Использование и быстрый реген ---
                if (!magicCooldown && Math.random() < 0.01 && magicCount > 0) {
                    magicCount--;
                    magicCooldown = true;
                    magicVal = 0.0;
                    console.log("🧪 Magick used! Count:", magicCount);
                }
                if (magicCooldown) {
                    magicVal += 0.01;
                    if (magicVal >= 1.0) {
                        magicVal = 1.0;
                        magicCount++;
                        magicCooldown = false;
                        console.log("✨ Magick restored! Count:", magicCount);
                    }
                }
                window.updateFlaskData(JSON.stringify({
                    typeIndex: 2,
                    percent: magicVal,
                    count: magicCount,
                    max_slots: 3,
                    forceGlow: false
                }));

            }, 16);

            console.groupEnd();
        }, 500); // Небольшая задержка перед стартом
    }
});
