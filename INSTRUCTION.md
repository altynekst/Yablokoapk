# Инструкция: залить сурс и получить APK (Yabloko, 64-бит)

## Что нужно скачать (один раз)

1. **Termux** — с F-Droid (https://f-droid.org/packages/com.termux/), НЕ из Google Play.
2. В Termux: `pkg update -y && pkg install git python`
3. Скрипт `upload_yablokoapk.py` — положи в `/storage/emulated/0/Download/`.
4. Токен GitHub (см. ниже).

**Больше ничего качать не надо.** Android SDK, NDK, Gradle, cmake — всё это
качает GitHub Actions на своих серверах. На телефоне ничего собирать не нужно.

## Токен GitHub

1. Открой https://github.com/settings/tokens
2. Generate new token (classic) -> отметь галку **repo** -> Generate.
3. Скопируй токен (начинается с `ghp_`). НИГДЕ его не публикуй и не вставляй в чаты.
   Если токен засветился в логе/чате — сразу удали его (Delete) и сделай новый.

## Как залить сурс

1. Сурс должен лежать в `/storage/emulated/0/Download/yabloko_src`
   (внутри: `CMakeLists.txt`, `src/`, `data/`, `scripts/android/`).
2. В Termux:
   ```
   python3 /storage/emulated/0/Download/upload_yablokoapk.py
   ```
3. Введи токен (ввод скрытый) и жди. Скрипт сам:
   - проверит репозиторий через GitHub API ДО пуша (чтобы не гнать ~94 МБ зря);
   - скопирует сурс в `~/yablokoapk_push`;
   - удалит штатные воркфлоу DDNet и поставит свой (сборка только arm64);
   - поправит брендинг (app_name -> Yabloko);
   - положит README.md, INSTRUCTION.md, .gitignore, .gitmodules;
   - сделает коммит и запушит в `altynekst/Yablokoapk`, ветка `main`.
4. Повторный запуск безопасен: если `~/yablokoapk_push` уже готова, скрипт предложит
   переиспользовать её — только обновит служебные файлы и запушит.

## Как получить APK

1. Открой https://github.com/altynekst/Yablokoapk/actions — после пуша там появится
   запуск **Android APK (arm64)**.
2. Жди ~40-70 минут.
3. APK: вкладка **Releases** -> `yabloko-v<N>` -> `Yabloko.apk` (arm64-v8a, только 64-бит).

## Частые ошибки

- **`Repository not found` при пуше** — неверное имя репозитория или токен без доступа.
  Правильный адрес: `https://github.com/altynekst/Yablokoapk` (Yabloko, не Ybloko).
- **`No such file or directory` при запуске `scripts/android/...` в Termux** — так и
  должно быть: эти скрипты нужны GitHub Actions (или ПК) и запускаются только из КОРНЯ
  сурса (`yabloko_src`), а не из Termux. Локальная сборка на телефоне не нужна.
- **Сборка Actions упала** — открой запуск, выбери красную ступень, посмотри лог;
  последняя ошибка обычно в конце.

## Обновление клиента

1. Правишь код/данные в `/storage/emulated/0/Download/yabloko_src`.
2. Снова запускаешь `upload_yablokoapk.py`.
3. Через ~40-70 минут в Releases появляется новый релиз `yabloko-v<N+1>`.
