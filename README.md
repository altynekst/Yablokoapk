# Yabloko — клиент DDNet для Android (только 64-бит)

Клиент Yabloko — модифицированный DDNet (19.8.2) для Android.
В этом репозитории лежит исходный код, APK собирается автоматически через GitHub Actions.

## Как получить APK

1. Любой пуш в ветку `main` запускает сборку автоматически.
2. Вкладка **Actions** -> workflow **Android APK (arm64)**.
3. Сборка занимает ~40-70 минут.
4. Готовый файл появится:
   - вкладка **Releases** -> релиз `yabloko-v<N>` -> `Yabloko.apk`;
   - вкладка **Actions** -> завершённый запуск -> **Artifacts** -> `Yabloko-android`.
5. Скачай `Yabloko.apk` на телефон и установи (разреши установку из неизвестных источников).

Собирается **только 64-бит (arm64-v8a)** — подходит для всех телефонов с Android 8.0+
на 64-битных процессорах (практически все современные телефоны).

Запустить сборку вручную: **Actions -> Android APK (arm64) -> Run workflow**.

## Что внутри (фичи Yabloko)

- Форк DDNet 19.8.2, брендинг «Yabloko | DDNet», иконка лаунчера «Yabloko»
- 10 компонентов: Background, Cosmetics, Dynamics, Toolkit, Presentation,
  Community, Extras, Zz, Pilot, AimVisuals
- 632 кастомные переменные (`cl_yabloko_*`, `cl_zz_*`, `krx_*`) — консоль (F1)
  и вкладка «Яблоко» в настройках
- 31 фон в `data/yabloko/backgrounds`, шрифты Inter
- Фикс переключения дамми-бота

## Структура

- `src/`, `data/`, `scripts/android/` — исходники DDNet/Yabloko
- `ddnet-libs` — сабмодуль (прекомпилированные библиотеки DDNet), CI скачивает сам
- `.github/workflows/android-apk.yml` — автосборка APK (только arm64)

## Локальная сборка

На телефоне в Termux собирать **не нужно и не получится** (нет Android SDK/NDK) —
APK собирает GitHub Actions. На ПК (Linux/macOS) сборка возможна:
`scripts/android/cmake_android.sh arm64 DDNet com.altynekst.yabloko Release build-android-release`
— но проще дождаться CI.
