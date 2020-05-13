Приложение NabletSolution реализует функции muxer, demuxer, encoder, decoder.

Приложение использует бибилиотеки:
nb_dec_aac.dll
nb_enc_aac.dll
nb_mp4_muxer.dll
nb_mp4demux.dll

скачать их можно с https://www.nablet.com/


Описание функционала

1. Поместите библиотеки в папку \NabletSolution\lib.
2. Выполните компиляцию решения NabletSolution.
3. Перейдите в папку \NabletSolution\Debug, Вам будут доступны следующие 4 программы.
4. demuxer.exe извелает треки из mp4 файла по номеру трека (0-31) и помещает в файл .\Debug\dump.tmp.
5. acc_dec.exe конвертирует acc в PCM.
6. acc_en.exe конвертируsт PCM в acc.
7. muxer.exe упаковывает дорожки в mp4 файл.
