
# Почему я не могу реализовать comet server на php?

Вы можете реализовать на php всё что угодно, в том числе и комет сервер или chat server. Но работать этот самопальный push сервис будет не так быстро как если этот же функционал реализовать на С++. И потребление ресурсов у программ на писаных на C++ обычно меньше.

Для примера если реализовать некий аналог комет сервера на php то для каждого клиента онлайн веб сервер apache создал бы по 1 потоку, и потратил бы не менее 6 - 10 мегабайт оперативной памяти на каждого человека онлайн, просто он предназначен для того чтобы быстро отдать контент страницы и обслуживать другой запрос а не висеть в памяти по несколько минут или десятки минут. И в виду одно поточного выполнения php скриптов реализовать быстрый push service на php вряд ли удастся. В то время как комет сервер специализирован для работы с большим количеством соединений единовременно.