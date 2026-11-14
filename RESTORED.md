# RESTORED.md — реконструкция исходников Yabloko

Этот файл описывает, как исходники клиента **Yabloko** были восстановлены по
единственному уцелевшему артефакту — PC-билду (`build/Yabloko.exe`, 942 файла).
Полная побайтовая верность исходников невозможна (сурс был утерян вместе с ПК):
это **аргументированная реконструкция**, где каждый элемент восстановлен по
следам в бинарнике и данных.

## Источники восстановления

| След | Что дал |
|---|---|
| Версия в строках билда | база **DDNet 19.8.2** (`source/`) |
| RTTI-имена классов в `Yabloko.exe` | архитектура: `CYablokoBackground`, `CYablokoCosmetics`, `CYablokoDynamics`, `CYablokoToolkit`, `CYablokoPresentation`, `CYablokoCommunity`, `CYablokoExtras`, `CYablokoAimVisuals`, `CYablokoMedia`, `YablokoNativeAvoid::CSimulation`, `CControls::ApplyYablokoSafety` |
| Путь в бинаре `C:\dev\ddnet-working-ok2\src\game\client\components\yabloko\visuals\media.cpp` | расположение и структура каталога `src/game/client/components/yabloko/` (восстановлен `visuals/media.cpp`) |
| Строки help-текстов конфиг-переменных в бинаре | **632 переменные** с оригинальными описаниями (386 `cl_yabloko_*`, 183 `cl_zz_*` от ZZetClient, 14 `krx_*` от KRX, прочие) |
| Строки бинда и консоли | `+yabloko_eye_laser`, `+yabloko_edit_hud`, `+yabloko_send_ads`, `+yabloko_emote_spam`, `yabloko_autopilot_toggle`, `yabloko_autopilot_goal`, `yabloko_autopilot_reset_goal`, `yabloko_chat_search`, `yabloko_random_copy`, `yabloko_reset_hud`, `yabloko_effects_settings`, `yabloko_users_settings` |
| Строки UI в бинаре | заголовок окна `Yabloko | DDNet`, ад-текст `я использую клиент Yabloko. скачать - t.me/DobriDDNnet`, метка клана `[Yabloko]`, страница настроек «Настройки → Яблоко» |
| `build/data` (байт-точно) | шейдеры (`gSunStrength`/`gDamageRects`), `tentacle_hook.png`, шрифты Inter, **31 фон тем** (`yabloko/backgrounds/*.png`), `sv_max_clients 128` и пр. |
| Кредитка `iamtater@sjrc6.github.io` в бинаре | вкомпилированный TaterClient: оттуда восстановлены `cl_back_button*` (порт из TaterClient-ddnet) и серверные `sv_*` лимитеры |
| `YablokoBot` в бинаре | дефолт имени ботов (`cl_zz_async_bots_base_name`) |

## Архитектура (src/game/client/components/yabloko/)

Имена классов 1-в-1 совпадают с RTTI из бинарника:

- **`CYablokoBackground`** — анимированный фон в игре: 31 тема из
  `data/yabloko/backgrounds/*.png`, параллакс, частицы, затемнение, цикл тем.
- **`CYablokoCosmetics`** — тентакль-хук (кагунэ: стиль/размер/волны/свечение/
  качество + `tentacle_hook.png`), снег, орбы, jelly, свечения (тело/тизи/заморозка),
  шляпа, поднимающийся снег, эффекты смерти/спавна, KRX-цвет хука.
- **`CYablokoDynamics`** — пружинная сетка (реагирует на игроков), плазма-щит,
  тесла-хук, стеклянный шаттер, afterburner, свечение тайлов заморозки.
- **`CYablokoToolkit`** — HUD: уведомления (вход/выход/упоминания/приваты),
  полоса времени хука, destra-панели (сервер/игроки/результаты/голосование/чат/музыка),
  радар, редактор HUD (+`yabloko_reset_hud`), виджет сервера, кнопка назад
  (механика портирована из TaterClient), список активных функций, оверлей ввода,
  предсказание пути.
- **`CYablokoPresentation`** — неоновый курсор, частицы/погода в меню, анимированная
  загрузка, тема (classic/violet/red/emerald/custom).
- **`CYablokoCommunity`** — метка `[Yabloko]` (публикуется вместо клана, сохранённый
  клан остаётся в настройках), подсветка игроков/серверов Yabloko, счётчик,
  библиотека `yabloko/library.json`, авто-висперы, реклама.
- **`CYablokoExtras`** — бинды `+yabloko_emote_spam`/`+yabloko_eye_laser`/
  `+yabloko_edit_hud`, emote-спам, `yabloko_random_copy`, KRX-идентичность
  (`krx_identity_save/load`), тайм-код чата.
- **`CYablokoZz`** — реконструкция ZZetClient-сьюта (183 переменные): аимбот,
  autohook, balance bot, fake lag/aim, hammer-gun, fast hammer, force zoom,
  hammer assist и др.
- **`CYablokoPilot`** — автопилот Race/Gores (цель через `yabloko_autopilot_goal`),
  избегание опасностей (`YablokoNativeAvoid::CSimulation`), sentry, FNG-ассистент,
  `CControls::ApplyYablokoSafety` — вето небезопасных вводов.
- **`CYablokoAimVisuals`** — ghost aura вокруг цели аимбота.
- **`CYablokoMedia`** (`visuals/media.cpp`) — звук приватных сообщений,
  медиа-взаимодействие.

Точка ввода: `CControls::SnapInput()` → `CYablokoZz::ApplyInput`,
`CYablokoPilot::ApplyInput`/`ApplySafety`, `CControls::ApplyYablokoSafety`.
Порядок рендера: фон до карты; эффекты мира после FreezeBars; HUD после Hud;
презентация перед меню. Клиент-маркер клана отправляется в
`CGameClient::SendStartInfo`/`ChangeInfo`.

## Точность реконструкции

**Восстановлено точно (байт/1-в-1):**
- все 632 кастомные конфиг-переменные с оригинальными именами и help-текстами;
- все кастомные ассеты `data/` (шейдеры, тентакль, фоны тем, шрифты Inter);
- брендинг, бинды, консольные команды, имена классов/файлов;
- `cl_back_button*` — работающий порт из TaterClient.

**Восстановлено функционально (работает, поведение близкое, не побайтовое):**
- косметика, фон, HUD-панели, уведомления, community, фичи ZZ/Pilot.

**Зарегистрировано, поведение упрощено или заглушка:**
- часть тяжёлой AI-логики ZZetClient/Pilot (async proxy bots, SOCKS5-прокси,
  fake ping, fly-помощник, companion AI, защита на всей глубине) — переменные
  есть в настройках, базовое поведение реализовано;
- `cl_yabloko_skin_3d` (3D-превью тизи), `cl_yabloko_media_interaction`
  (Windows SMTC), `cl_yabloko_font` (подмена шрифта), преднастройка
  `gDamageRects`-шейдерных индикаторов урона, поиск по чату, радар-стриминг
  позиций от сервера.

## Сборка

Требуется: CMake, Python 3, компилятор с C++20, Rust/Cargo, SDL2, freetype,
curl, sqlite3, libpng, zlib, brotli, opus/opusfile, wavpack, OpenSSL, ffmpeg (опц.).

```bash
cd source
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j4
# клиент: Yabloko | DDNet (бинарник ddnet/ddnet.exe)
```

Все кастомные переменные можно смотреть через `cl_...` в консоли/F1 или в
Настройки → Яблоко.
