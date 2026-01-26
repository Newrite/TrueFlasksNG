// =========================================================
// НАСТРОЙКИ ВИЗУАЛИЗАЦИИ
// =========================================================

// 0.0 - самый низ картинки SVG
// 1.0 - самый верх картинки SVG

// Уровень дна (откуда начинается жидкость)
const VISUAL_BOTTOM = 0.20;

// Уровень горлышка (где жидкость должна остановиться при 100%)
const VISUAL_TOP = 0.65;

// Внутренний отступ из CSS (30px), из-за которого виджет не прижимался к краю
// (Исправление координат)
const CSS_INTERNAL_PADDING = 60;

// =========================================================

const flasks = [
    // Добавлено поле groupId для надежного поиска враппера (Исправление позиционирования)
    {id: 'flask-health', groupId: 'flask-group-health', counterId: 'flask-counter-health', type: 0, color: '#6a0020'},
    {
        id: 'flask-stamina',
        groupId: 'flask-group-stamina',
        counterId: 'flask-counter-stamina',
        type: 1,
        color: '#2f7d33'
    },
    {id: 'flask-magick', groupId: 'flask-group-magick', counterId: 'flask-counter-magick', type: 2, color: '#2f4ba4'},
    {id: 'flask-other', groupId: 'flask-group-other', counterId: 'flask-counter-other', type: 3, color: '#7b4997'}
];

const flaskElements = {};
let globalSettings = {auto_hide: false, opacity: 1.0};

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
        // Находим враппер надежно по ID группы (Исправление позиционирования)
        const wrapperElement = document.getElementById(item.groupId);

        flaskElements[item.type] = {
            object: obj,
            wrapper: wrapperElement || obj.parentElement, // Fallback
            svgRoot: svgRoot,
            fillRect: svgDoc.getElementById('flask-fill-rect'),
            text: textElement,
            lastCount: -1,
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

// === Настройки позиционирования ===
window.setWidgetSettings = (settingsJson) => {
    try {
        const settings = JSON.parse(settingsJson);
        globalSettings = settings;
        const container = document.getElementById('widget-container');
        if (!container) return;

        container.style.display = settings.enable ? 'block' : 'none';
        if (!settings.enable) return;

        if (settings.anchor_all) {
            // === ИСПРАВЛЕНИЕ КООРДИНАТ ===
            // Вычитаем внутренний отступ CSS, умноженный на масштаб.
            const offset = CSS_INTERNAL_PADDING * settings.size;

            container.style.left = ((settings.x * window.innerWidth) - offset) + 'px';
            container.style.top = ((settings.y * window.innerHeight) - offset) + 'px';
            container.style.transform = `scale(${settings.size})`;
            container.style.opacity = settings.opacity;

            flasks.forEach(item => {
                // Используем groupId для поиска
                let wrapper = flaskElements[item.type]?.wrapper || document.getElementById(item.groupId);

                if (wrapper) {
                    const flaskSettings = (item.type === 0) ? settings.health : (item.type === 1) ? settings.stamina : (item.type === 2) ? settings.magick : settings.other;

                    // === ИСПРАВЛЕНИЕ ПОЗИЦИОНИРОВАНИЯ (Anchor All) ===
                    // Агрессивно удаляем стили, чтобы вернуть управление CSS-классам (.flask-top и т.д.)
                    wrapper.style.removeProperty('left');
                    wrapper.style.removeProperty('top');
                    wrapper.style.removeProperty('transform');
                    wrapper.style.removeProperty('position');

                    // Логика прозрачности с учетом auto_hide
                    if (!settings.auto_hide) {
                        wrapper.style.opacity = (flaskSettings && flaskSettings.enabled === false) ? '0' : '1';
                    } else {
                        // Если авто-скрытие включено, но мы еще не знаем статус (инициализация), скрываем
                        if (flaskElements[item.type] && flaskElements[item.type].maxSlots === -1) {
                            wrapper.style.opacity = '0';
                        }
                    }
                }
            });
        } else {
            // Режим раздельного позиционирования
            container.style.left = '0px';
            container.style.top = '0px';
            container.style.transform = 'scale(1)';
            container.style.opacity = '1';

            const apply = (type, s) => {
                let el = flaskElements[type]?.wrapper || document.getElementById(flasks.find(f => f.type === type)?.groupId);
                if (!el) return;

                if (s.enabled === false) {
                    el.style.opacity = '0';
                    return;
                }

                // Применяем жесткие координаты
                el.style.left = (s.x * window.innerWidth) + 'px';
                el.style.top = (s.y * window.innerHeight) + 'px';
                el.style.transform = `scale(${s.size})`;

                if (!settings.auto_hide) {
                    el.style.opacity = s.opacity;
                }
            };
            apply(0, settings.health);
            apply(1, settings.stamina);
            apply(2, settings.magick);
            apply(3, settings.other);
        }
    } catch (e) {
        console.error(e);
    }
};

window.setWidgetSettingsInit = (settingsJson) => setTimeout(() => window.setWidgetSettings(settingsJson), 1500);


// === ГЛАВНАЯ ЛОГИКА ОБНОВЛЕНИЯ ===
window.updateFlaskData = (args) => {
    if (!args) return;

    let flaskType, fillPercent, count, maxSlots, shouldGlow, animationFill, animationFillOnlyZero;

    try {
        const params = JSON.parse(args);
        flaskType = parseInt(params.typeIndex);
        fillPercent = parseFloat(params.percent);
        count = parseInt(params.count);
        maxSlots = parseInt(params.max_slots);
        shouldGlow = params.forceGlow;
        animationFill = params.fill_animation;
        animationFillOnlyZero = params.fill_animation_only_zero;
    } catch (e) {
        return;
    }

    const el = flaskElements[flaskType];
    if (!el) return;

    el.maxSlots = maxSlots;
    
    if (!animationFill) {
        fillPercent = 1.0;
    }
    
    if (animationFill && animationFillOnlyZero && count > 0) {
        fillPercent = 1.0;
    }

    // ---------------------------------------------------------
    // 1. Визуализация заполнения (Mapping)
    // ---------------------------------------------------------
    const visualRange = VISUAL_TOP - VISUAL_BOTTOM;
    const mappedScale = VISUAL_BOTTOM + (fillPercent * visualRange);

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

    if (el.lastCount === -1) {
        el.lastCount = count;
    }

    if (count > el.lastCount) {
        triggerGlow = true;
    }
    el.lastCount = count;

    if (triggerGlow) {
        el.object.classList.remove('glowing');
        void el.object.offsetWidth;
        el.object.classList.add('glowing');
    }

    // ---------------------------------------------------------
    // 4. Автоскрытие / Показ элемента
    // ---------------------------------------------------------
    if (el.wrapper) {
        let targetOpacity = '1';

        if (globalSettings.auto_hide) {
            // Скрываем, если полная фласка
            if (count >= maxSlots) {
                targetOpacity = '0';
            } else {
                targetOpacity = '1';
            }
            // Apply fade
            el.wrapper.style.transition = 'opacity 1.5s ease';
            el.wrapper.style.opacity = targetOpacity;
        } else {
            // Если автоскрытие выключено, убеждаемся, что элемент видим (если он не выключен в настройках)
            // Здесь мы доверяем тому, что setWidgetSettings уже задал правильную opacity
            // Но если элемент был скрыт анимацией авто-скрытия, а потом настройку выключили "на лету",
            // нужно вернуть ему видимость.
            // Простейшая проверка: если стиль opacity 0 или пуст, делаем 1 (сброс)
            if (el.wrapper.style.opacity === '0' && !el.wrapper.style.transition) {
                // Оставляем как есть, возможно выключен через конфиг
            } else if (el.wrapper.style.transition) {
                // Если осталась транзишн от авто-скрытия, убираем её и восстанавливаем
                el.wrapper.style.transition = '';
                el.wrapper.style.opacity = '1';
            }
        }
    }
};

// === ФУНКЦИИ УПРАВЛЕНИЯ ВИДИМОСТЬЮ (TOP LEVEL) ===
window.Hide = () => {
    document.body.style.transition = 'none';
    document.body.style.opacity = '0';
};

window.Show = () => {
    // Принудительный пересчет стилей
    void document.body.offsetWidth;
    document.body.style.transition = 'opacity 3s ease';
    document.body.style.opacity = '1';
};

document.addEventListener('DOMContentLoaded', () => {
    setTimeout(() => window.firstInitDom(), 1000);
});

// =========================================================
// DEBUG / BROWSER TESTING MODE
// =========================================================
const DEBUG_FLASKS = false;

window.addEventListener('load', function () {

    if (!DEBUG_FLASKS) return;

    window.Hide();
    window.Show();

    const isBrowser = (typeof window !== 'undefined' && (window.location.protocol === 'file:' || window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1'));

    if (isBrowser || (typeof FORCE_TEST !== 'undefined' && FORCE_TEST)) {
        console.group("%c TrueFlasks Debug Started ", "background: #222; color: #bada55; font-size: 14px");
        console.log("Environment detected: Browser/Debug");

        document.body.style.backgroundColor = "#1a1a1a";
        document.body.style.backgroundImage = "linear-gradient(45deg, #1a1a1a 25%, #2a2a2a 25%, #2a2a2a 50%, #1a1a1a 50%, #1a1a1a 75%, #2a2a2a 75%, #2a2a2a 100%)";
        document.body.style.backgroundSize = "20px 20px";

        const mockSettings = {
            enable: true,
            x: 0, y: 0, size: 1.0, opacity: 1.0,
            anchor_all: true,
            auto_hide: false,
            health: {enabled: true, x: 0.3, y: 0.4, size: 0.8, opacity: 1.0},
            stamina: {enabled: true, x: 0.4, y: 0.4, size: 0.8, opacity: 1.0},
            magick: {enabled: true, x: 0.5, y: 0.4, size: 0.8, opacity: 1.0},
            other: {enabled: true, x: 0.6, y: 0.4, size: 0.8, opacity: 1.0}
        };

        setTimeout(() => {
            const testObj = document.getElementById('flask-health');
            if (testObj) {
                try {
                    const doc = testObj.contentDocument;
                    if (!doc) {
                        console.error("❌ ОШИБКА ДОСТУПА К SVG");
                        return;
                    } else {
                        console.log("✅ Доступ к SVG есть.");
                    }
                } catch (e) {
                    console.error("Ошибка безопасности:", e);
                }
            }

            if (window.setWidgetSettings) {
                window.setWidgetSettings(JSON.stringify(mockSettings));
                window.firstInitDom();
            }

            let time = 0;
            let healthVal = 0.0;
            let magicVal = 1.0;
            let magicCount = 2;
            let magicCooldown = false;

            setInterval(() => {
                time += 16;
                healthVal += 0.005;
                if (healthVal > 1.3) healthVal = 0.0;

                let hPercent = Math.min(healthVal, 1.0);
                let hCount = healthVal >= 1.0 ? 1 : 0;

                window.updateFlaskData(JSON.stringify({
                    typeIndex: 0, percent: hPercent, count: hCount, max_slots: 1, forceGlow: false
                }));

                if (!magicCooldown && Math.random() < 0.01 && magicCount > 0) {
                    magicCount--;
                    magicCooldown = true;
                    magicVal = 0.0;
                }
                if (magicCooldown) {
                    magicVal += 0.01;
                    if (magicVal >= 1.0) {
                        magicVal = 1.0;
                        magicCount++;
                        magicCooldown = false;
                    }
                }
                window.updateFlaskData(JSON.stringify({
                    typeIndex: 2, percent: magicVal, count: magicCount, max_slots: 3, forceGlow: false
                }));
            }, 16);
            console.groupEnd();
        }, 500);

        // Тест переключения режимов
        setTimeout(() => {
            window.setWidgetSettings(JSON.stringify({...mockSettings, enable: false}))
        }, 2500);
        setTimeout(() => {
            window.setWidgetSettings(JSON.stringify({...mockSettings, anchor_all: false}))
        }, 3500); // Тест раздельного
        setTimeout(() => {
            window.setWidgetSettings(JSON.stringify({...mockSettings, anchor_all: true}))
        }, 5500); // Возврат к общему (проверка очистки стилей)
        setTimeout(() => {
            window.setWidgetSettings(JSON.stringify({...mockSettings, enable: true}))
        }, 4500);
    }
});
