1. ИСПОЛЬЗОВАНИЕ

Плагин проверен с Гимпом 2.6, GCC 4.2.4, GTK 2.12, на платформе x86.

Плагин состоит из одного файла plugin-img,
который надо скопировать в собственный каталог
плагинов Гимпа - ~/.gimp-2.6/plug-ins/. И всё,
Гимп будет открывать и сохранять файлы *.img.

Можно скопировать плагин и в системный
каталог (/usr/lib/gimp/2.0/plug-ins/).

Как и большинство файловых плагинов Гимпа,
этот привязан к расширению (IMG), поэтому файлы
с таким расширением будут загружаться и
сохраняться этим плагином. Описание MIME
сделано весьма условно (image/x-img),
сигнатура отсутствует (в гифах, например, есть слово GIF).

При открытии файла *.img плагин запоминает
его формат и прозрачный цвет, если такой имеется.
Поэтому если потом просто сохранить изменённое
изображение (жамкнув Ctrl+S или F2-как у меня),
то файл сохранится в том же формате.

Чтобы сохранить изображение в другом формате
надо нажать Shift+Ctrl+S (Shift+F2),
это пункт "Сохранить как..." в меню "Файл".
Тогда появится окошко с предложением выбрать формат
и прозрачный цвет. В этом окошке уже будут предложены
параметры: либо с последнего сохранения,
либо запомненные при открытии изображения.

Прозрачный цвет имеет смысл
только для форматов RGB и RGBA. В последнем случае
пиксели с совпадающем цветом становятся полностью прозрачными.
Выбрать прозрачный цвет можно щёлкнув мышкой
по миниатюре или по большому прямоугольнику :-)

Плагин умеет сохранять только изображения в 
формате RGB (с прозрачностью или без оной);
размеры всех слоёв должны совпадать с размерами
изображения. Если что-то не так плагин скажет об этом.

Пример неинтерактивной работы плагина показан в файле img-fu.



2. УСТАНОВКА

Плагин собирается из отдельных файлов *.c
Параметры компилятора и компоновщика
выясняются с помощью утилиты gimptool-2.0,
которая идёт вместе с Гимпом.
Всё это описано в файле Makefile,
так что собрать и установить (в ~/.gimp-2.6/plug-ins/)
плагин можно командой "make install".

Другие возможности Makefile покажет
команда "make help" и редактор vim.

Сборка для Винды не предусмотрена,
но я исследовал этот вопрос. В известной сборке
Гимпа для Винды нет даже заголовочных файлов.

Для отключения отладочных сообщений надо
закомментировать строку: #define DEBUG
в файле plugin-img.h

В файле img-fu пример скрипта на языке Scheme,
которые открывает файл и сохраняет его
в другом формате (не перезаписывая).
Проверить это можно командой
"make test-fu" или сразу "gimp -i -b - < img-fu"

Не понятно, зачем всем процедурам два раза
передавать имя файла, но таково устройство Гимпа.


Вот пример сборки и установки плагина
(я удалил предупреждения, связанные с исходниками Гимпа):

# make install
gcc -c -I/usr/include/gimp-2.0 -I/usr/include/gtk-2.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/pixman-1   -O2 -ansi -pedantic -Wall -Wextra -Wno-attributes -Wno-unused-parameter  img-load.c -o img-load.o
gcc -c -I/usr/include/gimp-2.0 -I/usr/include/gtk-2.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/pixman-1   -O2 -ansi -pedantic -Wall -Wextra -Wno-attributes -Wno-unused-parameter  img-save.c -o img-save.o
gcc -c -I/usr/include/gimp-2.0 -I/usr/include/gtk-2.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/pixman-1   -O2 -ansi -pedantic -Wall -Wextra -Wno-attributes -Wno-unused-parameter  img-save-dialog.c -o img-save-dialog.o
gcc -c -I/usr/include/gimp-2.0 -I/usr/include/gtk-2.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/pixman-1   -O2 -ansi -pedantic -Wall -Wextra -Wno-attributes -Wno-unused-parameter  plugin-img.c -o plugin-img.o
gcc -lgimpui-2.0 -lgimpwidgets-2.0 -lgimpmodule-2.0 -lgimp-2.0 -lgimpmath-2.0 -lgimpconfig-2.0 -lgimpcolor-2.0 -lgimpbase-2.0 -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lpangocairo-1.0 -lpango-1.0 -lcairo -lgobject-2.0 -lgmodule-2.0 -lglib-2.0   img-load.o img-save.o img-save-dialog.o plugin-img.o -o plugin-img
gimptool-2.0 --install-bin plugin-img
cp plugin-img /home/pashev/.gimp-2.6/plug-ins

