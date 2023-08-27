Split Browser
=============

Split Browser is a minimalistic, ultra-lightweight, open source web browser for desktop, based on [WebKit](https://webkit.org/) (provided by [Playwright](https://playwright.dev/)), [Ultralight](https://ultralig.ht/) and a native [webview](https://webview.dev/) (WebKit on macOS, WebKitGTK on Linux, Edge WebView2 on Windows), with split screen (tiled) view, made with [Qt](https://www.qt.io/).

![Split Browser](https://i.ibb.co/rcXz8Yd/Split-Browser.webp)

Every variant of Split Browser (WebKit, Ultralight, native webview) uses <100 MB of RAM to show the home page, which is much less than Chrome/Firefox/Vivaldi/Opera/Edge. It also allows you to show web pages side-by-side as tiles by dragging and dropping tabs.

See the RAM usage of all 3 variants showing the DuckDuckGo home page:

![Split Browser RAM usage](https://i.imgur.com/LbHUr1N.png)

Split Browser is in the alpha stage and for now it provides only basic web browsing features - no bookmarks, no history, no extensions, no adblock, no advanced settings.

## Download

You can download the latest binary for Windows 10+ in the [releases](https://github.com/niutech/splitbrowser/releases). Keep in mind, this is an alpha version, so use at your own risk!

## Build

This project is being developed on Windows 10 using Qt 5.14, MSVC 2017 and qmake, but it also runs on Linux and macOS, provided that you have downloaded the lastest [WebKitGTK](https://webkitgtk.org/), [Ultralight SDK](https://github.com/ultralight-ux/Ultralight#eyes-getting-the-latest-sdk) or [Playwright WebKit binaries](https://github.com/microsoft/playwright) for your platform. Extract them to folders: `../ultralight` and `../webkit` relative to `splitbrowser`, open this project in Qt Creator, set the engine in `splitbrowser.pro` and run it.

Here is Split Browser using WebKitGTK on Linux:

![Split Browser Linux Native](https://github.com/niutech/splitbrowser/assets/384997/81e56f29-a50d-47ab-9ae7-a0a78cdd5626)


## Contribute

Contributions are welcome!

## License

Split Browser &copy; 2022 Jerzy Głowacki under MIT License.

Ultralight &copy; 2022 Ultralight Inc. under [Ultralight Free License Agreement](https://github.com/ultralight-ux/Ultralight/blob/master/license/LICENSE.txt).

Playwright &copy; 2022 Microsoft Corp. under Apache 2.0 License.

Webview &copy; 2022 Serge Zaitsev et. al. under MIT license.
