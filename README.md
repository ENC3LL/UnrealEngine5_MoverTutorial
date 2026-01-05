# 🏃 Realistic Movement System (Mover 2.0)

![Unreal Engine](https://img.shields.io/badge/Unreal_Engine-5.7%2B-black?style=for-the-badge&logo=unrealengine)
![C++](https://img.shields.io/badge/Language-C%2B%2B-blue?style=for-the-badge&logo=c%2B%2B)
![Mover](https://img.shields.io/badge/System-Mover_2.0-orange?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

[English](#english) | [Русский](#russian)

---

<a name="english"></a>
## 🇬🇧 English Description

A production-ready **Physical Movement System** built on top of Unreal Engine 5's new **Mover 2.0** (Network Prediction) plugin. 

Unlike the standard `CharacterMovementComponent`, this project implements **physics-based locomotion** with real inertia, friction, and momentum conservation. It serves as a plug-and-play template for developers who want "AAA-feeling" movement without writing the boilerplate code from scratch.

### ⚡ Key Features
* **Physics-Based Walking:** Authentic acceleration, braking, and ground friction logic.
* **Inertia & Momentum:** Movement doesn't stop instantly; it respects mass and surface properties.
* **Modular Architecture:** Clean separation of `WalkMode` and `AirMode` using the new Mover 2.0 standard.
* **Ready-to-Use Character:** Includes a pre-configured Character class acting as a base.
* **Blueprint Exposed:** Fine-tune Mass, Gravity, and Friction directly in the Editor.

### 📂 Project Structure
* `RealisticMoverComponent` - The core brain replacing the old movement component.
* `RealisticWalkMode` - Handles ground locomotion, sprint, crouch, and sliding.
* `RealisticAirMode` - Advanced air physics, drag, and gravity scaling.
* `RealisticMovementSettings` - Data Asset for tweaking physics without code.
* `RealisticDataModel` - Input structures and network serialization.

### 🚀 Installation

1.  **Prerequisites:**
    Ensure the **Mover** plugin is enabled in your `.uproject` file.

2.  **Clone the repository:**
    ```bash
    git clone [https://github.com/ENC3LL/UnrealEngine5_MoverTutorial.git](https://github.com/ENC3LL/UnrealEngine5_MoverTutorial.git)
    ```

3.  **Integration:**
    * Copy the `Source` files into your project's C++ directory.
    * Right-click your `.uproject` and select **"Generate Visual Studio project files"**.
    * Compile the project.

### 🛠 Usage

**Option A: Use the Reference Character**
Simply drop the provided `BP_RealisticCharacter` (if you created one based on the C++ class) into the world.

**Option B: Add to Existing Character**
1.  Open your Character Blueprint.
2.  Add the `RealisticMover` component.
3.  Disable or ignore the default `CharacterMovement`.
4.  Configure the **Realistic Settings** in the Details panel.

### ⚙️ Configuration
You can tweak the feel of the movement via the `RealisticSettings` block in the component:

| Parameter | Recommended | Description |
| :--- | :--- | :--- |
| **Mass** | `80.0` | Mass of the character in kg. Affects inertia. |
| **Gravity Scale** | `2.0` | Multiplier for gravity. 2.0 feels snappier for action games. |
| **Ground Friction** | `8.0` | Lower values (e.g., 1.0) create an "icy" sliding effect. |
| **Braking Decel** | `2048.0` | How fast the character stops when input is released. |

---

<a name="russian"></a>
## 🇷🇺 Описание на Русском

Готовая к использованию **система физического передвижения**, построенная на новом плагине **Mover 2.0** (Network Prediction) для Unreal Engine 5.

В отличие от стандартного `CharacterMovementComponent`, этот проект реализует **физически корректную локомоцию** с инерцией, трением и сохранением импульса. Это "drop-in" решение для тех, кто хочет получить качественное передвижение персонажа, не переписывая базовую архитектуру с нуля.

### ⚡ Особенности
* **Физическая ходьба:** Честный расчет ускорения, торможения и трения о поверхность.
* **Инерция:** Персонаж не останавливается мгновенно, его "заносит" в зависимости от массы и покрытия.
* **Модульная архитектура:** Чистое разделение логики на `WalkMode` и `AirMode` (стандарт Mover 2.0).
* **Готовый персонаж:** Включает настроенный класс Character для быстрого старта.
* **Настройка в Blueprint:** Все параметры (Масса, Гравитация, Скорость) доступны в редакторе.

### 📂 Структура проекта
* `RealisticMoverComponent` - Основной компонент, заменяющий старый мувмент.
* `RealisticWalkMode` - Логика ходьбы, спринта, приседа и скольжения.
* `RealisticAirMode` - Физика полета, сопротивление воздуха и гравитация.
* `RealisticMovementSettings` - Класс настроек для управления физикой без кода.
* `RealisticDataModel` - Структуры ввода и сериализация данных для сети.

### 🚀 Установка

1.  **Требования:**
    Убедитесь, что плагин **Mover** включен в вашем `.uproject`.

2.  **Клонирование:**
    ```bash
    git clone [https://github.com/ENC3LL/UnrealEngine5_MoverTutorial.git](https://github.com/ENC3LL/UnrealEngine5_MoverTutorial.git)
    ```

3.  **Интеграция:**
    * Скопируйте файлы из `Source` в папку C++ вашего проекта.
    * Нажмите ПКМ на `.uproject` и выберите **"Generate Visual Studio project files"**.
    * Скомпилируйте проект.

### 🛠 Использование

**Вариант А: Готовый персонаж**
Используйте класс `RealisticCharacter` (который есть в файлах) как родительский класс для вашего Blueprint персонажа.

**Вариант Б: Добавление в своего персонажа**
1.  Откройте блюпринт вашего персонажа.
2.  Добавьте компонент `RealisticMover`.
3.  Отключите стандартный `CharacterMovement` (Auto Activate = false).
4.  Настройте параметры в секции **Realistic Settings**.

### ⚙️ Настройка
Вы можете менять физику через блок `RealisticSettings` в компоненте:

| Параметр | Значение | Описание |
| :--- | :--- | :--- |
| **Mass** | `80.0` | Масса персонажа в кг. Влияет на инерцию разгона. |
| **Gravity Scale** | `2.0` | Множитель гравитации. 2.0 делает прыжки более резкими (как в шутерах). |
| **Ground Friction** | `8.0` | Трение. Низкие значения (напр. 1.0) создают эффект льда. |
| **Braking Decel** | `2048.0` | Сила торможения при отпускании кнопок. |

---
*Created by [ENC3LL](https://github.com/ENC3LL)*
![photo_2025-09-22_11-59-25](https://github.com/user-attachments/assets/e65c5a45-c4cc-4fee-a13d-1d745910f0bf)
