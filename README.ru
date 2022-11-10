1. ИСПОЛЬЗОВАНИЕ

Плагин проверен с Гимпом 2.10, GCC 12, GTK 2.24, на платформе amd64.

Плагин состоит из одного файла plugin-img, который надо скопировать в собственный каталог
плагинов Гимпа - ~/.config/GIMP/2.10/plug-ins/plugin-img/. И всё, Гимп будет открывать и сохранять файлы *.img.

Как и большинство файловых плагинов Гимпа, этот привязан к расширению (IMG), поэтому
файлы с таким расширением будут загружаться и сохраняться этим плагином. Описание MIME
сделано весьма условно (image/x-img), сигнатура отсутствует (в гифах, например, есть слово GIF).

При открытии файла *.img плагин запоминает его формат и прозрачный цвет, если такой
имеется.  Поэтому если потом просто сохранить изменённое изображение (жамкнув Ctrl+S или
F2-как у меня), то файл сохранится в том же формате.

Чтобы сохранить изображение в другом формате надо нажать Shift+Ctrl+S (Shift+F2), это пункт
"Сохранить как..." в меню "Файл".  Тогда появится окошко с предложением выбрать формат
и прозрачный цвет. В этом окошке уже будут предложены параметры: либо с последнего
сохранения, либо запомненные при открытии изображения.

Прозрачный цвет имеет смысл только для форматов RGB и RGBA. В последнем случае пиксели с
совпадающем цветом становятся полностью прозрачными.  Выбрать прозрачный цвет можно
щёлкнув мышкой по миниатюре или по большому прямоугольнику :-)

Плагин умеет сохранять только изображения в формате RGB (с прозрачностью или без оной);
размеры всех слоёв должны совпадать с размерами изображения. Если что-то не так плагин
скажет об этом.

Пример неинтерактивной работы плагина показан в файле img-fu.



2. УСТАНОВКА

Плагин собирается из отдельных файлов *.c Параметры компилятора и компоновщика выясняются
с помощью утилиты gimptool-2.0, которая идёт вместе с Гимпом.  Всё это описано в файле Makefile.

Для отключения отладочных сообщений надо закомментировать строку: #define DEBUG в файле plugin-img.h

В файле img-fu пример скрипта на языке Scheme, которые открывает файл и сохраняет его в другом
формате (не перезаписывая).  Проверить это можно командой "make test-fu" или сразу "gimp -i -b - < img-fu"


Вот пример сборки и установки плагина:

$ make install
gcc -c -pthread -I/usr/include/gimp-2.0 -I/usr/include/gegl-0.4 -I/usr/include/gio-unix-2.0 -I/usr/include/json-glib-1.0 -I/usr/include/babl-0.1 -I/usr/include/gtk-2.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/x86_64-linux-gnu -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/fribidi -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16 -O2 -Wall -Wextra -Wno-attributes -Wno-unused-parameter img-load.c -o img-load.o
gcc -c -pthread -I/usr/include/gimp-2.0 -I/usr/include/gegl-0.4 -I/usr/include/gio-unix-2.0 -I/usr/include/json-glib-1.0 -I/usr/include/babl-0.1 -I/usr/include/gtk-2.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/x86_64-linux-gnu -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/fribidi -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16 -O2 -Wall -Wextra -Wno-attributes -Wno-unused-parameter img-save.c -o img-save.o
gcc -c -pthread -I/usr/include/gimp-2.0 -I/usr/include/gegl-0.4 -I/usr/include/gio-unix-2.0 -I/usr/include/json-glib-1.0 -I/usr/include/babl-0.1 -I/usr/include/gtk-2.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/x86_64-linux-gnu -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/fribidi -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16 -O2 -Wall -Wextra -Wno-attributes -Wno-unused-parameter img-save-dialog.c -o img-save-dialog.o
gcc -c -pthread -I/usr/include/gimp-2.0 -I/usr/include/gegl-0.4 -I/usr/include/gio-unix-2.0 -I/usr/include/json-glib-1.0 -I/usr/include/babl-0.1 -I/usr/include/gtk-2.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/x86_64-linux-gnu -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/fribidi -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16 -O2 -Wall -Wextra -Wno-attributes -Wno-unused-parameter plugin-img.c -o plugin-img.o
gcc img-load.o img-save.o img-save-dialog.o plugin-img.o -lgimpui-2.0 -lgimpwidgets-2.0 -lgimpmodule-2.0 -lgimp-2.0 -lgimpmath-2.0 -lgimpconfig-2.0 -lgimpcolor-2.0 -lgimpbase-2.0 -lgegl-0.4 -lgegl-npd-0.4 -Wl,--export-dynamic -lgmodule-2.0 -pthread -ljson-glib-1.0 -lbabl-0.1 -lgtk-x11-2.0 -lgdk-x11-2.0 -lpangocairo-1.0 -latk-1.0 -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lpangoft2-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lharfbuzz -lfontconfig -lfreetype -o plugin-img
gimptool-2.0 --install-bin plugin-img
cp 'plugin-img' '/home/pashev/.config/GIMP/2.10/plug-ins/plugin-img/'

