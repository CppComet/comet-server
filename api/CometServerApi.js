/**
 * JavaScript API for comet-server.com
 * I will be glad to new orders for something a development.
 *
 * @author Trapenok Victor (Трапенок Виктор Викторович), Levhav@ya.ru, 89244269357
 * Буду рад новым заказам на разработку чего нибудь.
 *
 * Levhav@ya.ru
 * Skype:Levhav
 * 89244269357
 * @link http://comet-server.com
 *
 * (҂`_´)
 * <,︻╦╤─ ҉ - - - - - - --
 * _/﹋\_
 *
 */


function getCookie(name)
{
     var cookie = " " + document.cookie;
     var search = " " + name + "=";
     var setStr = null;
     var offset = 0;
     var end = 0;
     if (cookie.length > 0) {
             offset = cookie.indexOf(search);
             if (offset != -1) {
                     offset += search.length;
                     end = cookie.indexOf(";", offset)
                     if (end == -1) {
                             end = cookie.length;
                     }
                     setStr = unescape(cookie.substring(offset, end));
             }
     }
     return(setStr);
}


function comet_server_signal()
{
    if(this.init === undefined) this.init = false;
    return comet_server_signal;
}

comet_server_signal.slotArray = new Array();
comet_server_signal.debug = false;

comet_server_signal.sigId = 0;

comet_server_signal.tabUUID = undefined;
comet_server_signal.getTabUUID = function()
{
    if(!comet_server_signal.tabUUID)
    {
        comet_server_signal.tabUUID = "";
        for(var i = 0; i< 16; i++)
        {
            comet_server_signal.tabUUID += "qwertyuiopasdfghjklzxcvbnm1234567890QWERTYUIOPASDFGHJKLZXCVBNM"[Math.floor(Math.random()*62)];
        }
    }
    return comet_server_signal.tabUUID;
}

/**
 * Подписывает слот на сигнал
 *
 * Если передать два параметра то они обработаются как  connect( signal_name, slot_function )
 * Если передать три параметра то они обработаются как  connect( slot_name, signal_name, slot_function )
 *
 * @param slot_name Имя слота
 * @param signal_name Имя сигнала
 * @param slot_function Функция вызваемая при вызове слота, должна иметь следующию сигнатуру function(param, signal_name){}
 *
 * <code>
 * Пример использования
 * new new signal().emit("catalogControl.OpenObject",{})
 *
 * </code>
 */
comet_server_signal.connect = function(slot_name, signal_name, slot_function)
{
    if(slot_function === undefined)
    {
        slot_function = signal_name;
        signal_name = slot_name;
        slot_name = "sig" + (comet_server_signal.sigId++)
    }

    if (comet_server_signal.slotArray[signal_name] === undefined)
    {
        comet_server_signal.slotArray[signal_name] = {}
    }
    comet_server_signal.slotArray[signal_name][slot_name] = slot_function;
    if(comet_server_signal.debug) console.log("На прослушивание сигнала " + signal_name + " добавлен слот " + slot_name + "", comet_server_signal.slotArray)
    return slot_name;
}


/**
 * Отписывает слот slot_name от сигнала signal_name
 */
comet_server_signal.disconnect = function(slot_name, signal_name)
{
    if (comet_server_signal.slotArray[signal_name] !== undefined)
    {
        if (comet_server_signal.slotArray[signal_name][slot_name] !== undefined)
        {
            comet_server_signal.slotArray[signal_name][slot_name] = undefined;
            return true
        }
    }
    return false
}

/**
 * Вызывает слоты подписаные на сигнал signal_name и каждому из них передаёт аруметы signal_name - имя вызвавшего сигнала, и param - объект с параметрами для слота)
 * В добавок ретранслирует сигнал в дочернии iframe если они есть и в родительское окно если оно есть
 * @param signal_name Имя сигнала
 * @param param Параметры переданые слоту при вызове в втором аргументе
 * @param SignalNotFromThisTab Если false то значит это сигнал пришёл из другой вкладки
 */
comet_server_signal.emit = function(signal_name, param, SignalNotFromThisTab)
{
    if (comet_server_signal.slotArray[signal_name] === undefined)
    {
        if(comet_server_signal.debug) console.log("На сигнал " + signal_name + " нет подписчиков")
    }
    else
    {
        if(comet_server_signal.debug) console.log("Сигнал " + signal_name + " подписаны слоты")
        var obj = comet_server_signal.slotArray[signal_name];
        for (var slot in obj)
        {
            if( obj.hasOwnProperty(slot) &&  obj[slot] !== undefined)
            {
                obj[slot](param,signal_name, SignalNotFromThisTab === true)
            }
        }

    }
}

/*
 *  генерация события будут оповещены и соседние вкладки
 *  @eName string - имя события
 *  использование .emit('любое название события', [ Параметры события ])
 */
comet_server_signal.emitAll = function (signal_name, param)
{
    comet_server_signal.emit(signal_name, param)

    try{
        if(window['localStorage'] !==undefined  )
        {
            var curent_custom_id = Math.random()+"_"+Math.random()+"_"+Math.random()+"_"+Math.random()+"_"+Math.random()
            window['localStorage']['comet_server_signal_storage_emit']= JSON.stringify({name:signal_name, custom_id:curent_custom_id, param:param});
        }
        return true
    }catch (e){
        return false
    }
}

/**
 * Запись состояния общего для всех вкладок
 * @param {string} name
 * @param {object} value
 * @param {number} minTime минимальный возраст данных меньше которого данные перезаписватся не должны в том случаии если они записанны не этой вкладкой
 */
comet_server_signal.setState = function(name, value, minTime)
{
    console.log("setState", name, value, minTime)
    var time = new Date()
    try{ 
        if(minTime)
        {
            var value = window.localStorage["tabSignal_"+name];
            if(value)
            {
                var val = JSON.parse(value)

                if(val.time + minTime > time.getTime() && val.tabUUID != comet_server_signal.getTabUUID() )
                {
                    // Возраст данных меньше minTime и они записаны не этой вкладкой, а значит мы их перезаписывать не будем
                    return false
                }
            }
        }

        window.localStorage["tabSignal_"+name] = JSON.stringify({time: time.getTime(), value: value, tabUUID: comet_server_signal.getTabUUID()})
        return true
    }catch (e){
        return false
    }
}

/**
 * Обновление с интервалом данных чтоб их не перезаписала другая вкладка
 * @param {type} name
 * @param {type} value
 * @param {type} minTime
 * @returns {undefined}
 */
comet_server_signal.intervalUpdateState = function(name, value, minTime)
{
    if(comet_server_signal.setState(name, value, minTime))
    {
        return setInterval(comet_server_signal.setState, minTime/3, name, value, minTime)
    }
    return undefined
}
/**
 * Чтение состояния общего для всех вкладок
 * @param {string} name
 * @param {number} maxTime Максимальный возраст данных в милесекундах после чего они считаются не действительными.
 * @returns {Window.localStorage}
 */
comet_server_signal.getState = function(name, maxTime)
{
    try{ 
        var time = new Date()
        var value = window.localStorage["tabSignal_"+name];
        if(value)
        {
            var val = JSON.parse(value)

            if(!maxTime)
            {
                // Нам не важен возраст данных
                return val.value
            }

            if(val.time + maxTime > time.getTime())
            {
                // Возраст данных меньше maxTime
                return val.value
            }
            return undefined
        } 
    }catch (e){ }
    return undefined
}

/**
 * Для совместимости с прошлой версией.
 *
 * Библиотека TabSignal.js (https://github.com/Levhav/TabSignal.js) полностью реализована
 * объектом comet_server_signal так как является составной частью JavaScript CometServerApi
 */
tabSignal = comet_server_signal;
comet_server_signal.send_emit = comet_server_signal.emitAll; // Для совместимости с прошлой версией.


if(!comet_server_signal.prototype.init)
{
    comet_server_signal.prototype.init = true
    if( window.addEventListener )
    {
        window.addEventListener('storage', function(e)
        {
            if(e.key && e.key == 'comet_server_signal_storage_emit')
            {// !testThis
                try{
                    var data = JSON.parse(e.newValue);
                    if(data !== undefined && data.name !== undefined  )
                    {
                        if(comet_server_signal.debug > 1) console.log( data )
                        comet_server_signal().emit( data.name, data.param, true )
                    }
                }
                catch (failed)
                {
                }
            }
        }, false);
    }
    else
    {
        document.attachEvent('onstorage', function(e)
        {
            if(e.key && e.key == 'comet_server_signal_storage_emit')
            {// !testThis
                try{
                    var data = JSON.parse(e.newValue);
                    if(data !== undefined && data.name !== undefined  )
                    {
                        if(comet_server_signal.debug > 1) console.log( data )
                        comet_server_signal().emit( data.name, data.param, true )
                    }
                }
                catch (failed)
                {
                }
            }
        } );
    }
}


var _cometServerApi = function(opt)
{
    if(opt)
    {
        if(_cometServerApi.prototype.options === undefined)
        {
            _cometServerApi.prototype.options = {};
        }

        for(var key in opt)
        {
            _cometServerApi.prototype.options[key] = opt[key];
        }
    }
    return this;
}

/**
 * @private
 */
_cometServerApi.prototype.version = "3.25";

/**
 * @private
 */
_cometServerApi.prototype.options = {};

/**
 * @private
 */
_cometServerApi.prototype.options.nodeArray = ["app.comet-server.ru"]// ["n1-app.comet.su", "n2-app.comet.su"]; //
_cometServerApi.prototype.options.node = undefined

/**
 * @private
 */
_cometServerApi.prototype.is_master = undefined;

/**
 * @private
 */
_cometServerApi.prototype.in_conect_to_server = false;

/**
 * @private
 */
_cometServerApi.prototype.in_try_conect = false;

/**
 * Массив имён каналов на которые мы подписаны
 * @private
 */
_cometServerApi.prototype.subscription_array = new Array();

/**
 * Случайный идентификатор вкладки.
 * Используется для определения кому предназначены исторические данные из канала.
 * @private
 */
_cometServerApi.prototype.custom_id = (Math.random()*10)+""+Math.random();
_cometServerApi.prototype.custom_id = _cometServerApi.prototype.custom_id.replace(/[^0-9A-z]/,"").replace(/^(.{10}).*$/,"$1");


/**
 * Время на переподключение в милисекундах после второй подряд ошибки подключения
 * @private
 */
_cometServerApi.prototype.time_to_reconect_on_error = [];

/**
 * Время на переподключение в милисекундах после первой ошибки подключения
 * @private
 */
_cometServerApi.prototype.time_to_reconect_on_close = [];

/**
 * @private
 */
_cometServerApi.prototype.in_abort = false;

/**
 * @private
 */
_cometServerApi.prototype.restart_time_id = false;

/**
 * Время даваемое на определение того какая из вкладок является мастервкладкой
 * @private
 */
_cometServerApi.prototype.start_timer = 1200;

/**
 * Выражение отделяющие по знаку точки на павую и левую части.
 * @private
 */
_cometServerApi.prototype.reg_exp = new RegExp(/^([^.]+)\.([^.]+)$/);

/**
 * Определяет надо ли использовать https или http
 * @private
 */
_cometServerApi.prototype.protocol = document.location.protocol.replace(/[^s]/img, "");

/**
 * @private
 */
_cometServerApi.prototype.web_socket_error = [];

/**
 * @private
 */
_cometServerApi.prototype.isSendStatisticsData = []
/**
 * Учитывает удачно переданные сообщения по вебскокету
 * Если они были то в случаии неполадок с ссетью переход на long poling осуществлён не будет.
 * @private
 */
_cometServerApi.prototype.web_socket_success = false;

/**
 * @private
 */
_cometServerApi.prototype.web_socket_error_timeOut = 30000;

/**
 * @private
 */
_cometServerApi.prototype.xhr_error = 0;
/**
 * @private
 */
_cometServerApi.prototype.xhr_error_timeOut_id = 30000;

/**
 * @private
 */
_cometServerApi.prototype.authorized_status;

/**
 * @private
 */
_cometServerApi.prototype.socket;
_cometServerApi.prototype.socketArray = [];

/**
 * @private
 */
_cometServerApi.prototype.use_WebSocket;

/**
 * @private
 */
_cometServerApi.prototype.request;

/**
 * @private
 */
_cometServerApi.prototype.status;

/**
 * @private
 */
_cometServerApi.prototype.send_msg_queue = [];

/**
 * содержит пакет данных о подписках готовый к отправке по вебсокету
 * @private
 * @type {string}
 */
_cometServerApi.prototype.send_msg_subscription = false;

/**
 * Уровень логирования
 * @private
 */
_cometServerApi.prototype.LogLevel = 0;

/**
 * Список уникальных идентификаторов сообщений которые были приняты
 * Используется чтоб гарантировать отсутсвие дублей
 * @type object
 */
_cometServerApi.prototype.msgsUUIDs = {}

try{
    if(window['localStorage']['comet_LogLevel'])
    {
        _cometServerApi.prototype.LogLevel = window['localStorage']['comet_LogLevel']/1
    }
}catch (e){}

_cometServerApi.prototype.getLogLevel = function()
{
    return _cometServerApi.prototype.LogLevel;
}

_cometServerApi.prototype.setLogLevel = function(level)
{
    _cometServerApi.prototype.LogLevel = level;
    try{
        window['localStorage']['comet_LogLevel'] = level;
    }catch (e){}
}

/** 
 * @returns {String} Сдучайная строка ровно из 10 символов
 */
_cometServerApi.prototype.getCustomString = function()
{
    var custom = (Math.random()*10)+""+Math.random();
    return custom.replace(/[^0-9A-z]/,"").replace(/^(.{10}).*$/,"$1");
}

/**
 * @returns {string} Уникальный ( с большой долей вероятности ) идентификатор вкладки
 */
_cometServerApi.prototype.getTabUUID = function()
{
    return comet_server_signal().getTabUUID()
}

/**
 *  http://www.webtoolkit.info/
 **/
_cometServerApi.prototype.Base64 = {
    _keyStr : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",
    encode : function (input) {
            var output = "";
            var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
            var i = 0;

            input = input.replace(/\r\n/g,"\n");
            var utftext = "";

            for (var n = 0; n < input.length; n++)
            {
                    var c = input.charCodeAt(n);
                    if (c < 128) {
                            utftext += String.fromCharCode(c);
                    }
                    else if((c > 127) && (c < 2048)) {
                            utftext += String.fromCharCode((c >> 6) | 192);
                            utftext += String.fromCharCode((c & 63) | 128);
                    }
                    else {
                            utftext += String.fromCharCode((c >> 12) | 224);
                            utftext += String.fromCharCode(((c >> 6) & 63) | 128);
                            utftext += String.fromCharCode((c & 63) | 128);
                    }
            }

            while (i < utftext.length) {

                    chr1 = utftext.charCodeAt(i++);
                    chr2 = utftext.charCodeAt(i++);
                    chr3 = utftext.charCodeAt(i++);

                    enc1 = chr1 >> 2;
                    enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
                    enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
                    enc4 = chr3 & 63;

                    if (isNaN(chr2)) {
                            enc3 = enc4 = 64;
                    } else if (isNaN(chr3)) {
                            enc4 = 64;
                    }
                    output = output +
                    this._keyStr.charAt(enc1) + this._keyStr.charAt(enc2) +
                    this._keyStr.charAt(enc3) + this._keyStr.charAt(enc4);
            }
            return output;
    },

    decode : function (input) {
                var output = "";
                var chr1, chr2, chr3;
                var enc1, enc2, enc3, enc4;
                var i = 0;

                input = input.replace(/[^A-Za-z0-9\+\/\=]/g, "");

                while (i < input.length) {

                        enc1 = this._keyStr.indexOf(input.charAt(i++));
                        enc2 = this._keyStr.indexOf(input.charAt(i++));
                        enc3 = this._keyStr.indexOf(input.charAt(i++));
                        enc4 = this._keyStr.indexOf(input.charAt(i++));

                        chr1 = (enc1 << 2) | (enc2 >> 4);
                        chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
                        chr3 = ((enc3 & 3) << 6) | enc4;

                        output = output + String.fromCharCode(chr1);

                        if (enc3 != 64) {
                                output = output + String.fromCharCode(chr2);
                        }
                        if (enc4 != 64) {
                                output = output + String.fromCharCode(chr3);
                        }

                }

                var string = "";
                var i = 0;
                var c = c1 = c2 = 0;

                while ( i < output.length ) {

                        c = output.charCodeAt(i);

                        if (c < 128) {
                                string += String.fromCharCode(c);
                                i++;
                        }
                        else if((c > 191) && (c < 224)) {
                                c2 = output.charCodeAt(i+1);
                                string += String.fromCharCode(((c & 31) << 6) | (c2 & 63));
                                i += 2;
                        }
                        else {
                                c2 = output.charCodeAt(i+1);
                                c3 = output.charCodeAt(i+2);
                                string += String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
                                i += 3;
                        }

                }

                return string;
        }
}

_cometServerApi.prototype.stripslashes = function(str)
{
    //       discuss at: http://phpjs.org/functions/stripslashes/
    //      original by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
    //      improved by: Ates Goral (http://magnetiq.com)
    //      improved by: marrtins
    //      improved by: rezna
    //         fixed by: Mick@el
    //      bugfixed by: Onno Marsman
    //      bugfixed by: Brett Zamir (http://brett-zamir.me)
    //         input by: Rick Waldron
    //         input by: Brant Messenger (http://www.brantmessenger.com/)
    // reimplemented by: Brett Zamir (http://brett-zamir.me)
    //        example 1: stripslashes('Kevin\'s code');
    //        returns 1: "Kevin's code"
    //        example 2: stripslashes('Kevin\\\'s code');
    //        returns 2: "Kevin\'s code"

    return (str + '')
      .replace(/\\(.?)/g, function(s, n1) {
        switch (n1) {
          case '\\':
            return '\\';
          case '0':
            return '\u0000';
          case '':
            return '';
          default:
            return n1;
        }
      });
}
/**
 * Выполняет привязку callBack функции к событию.
 * И при происшествии события на которое мы подписывались в функции subscription
 * определяет надо ли дёргать callBack функцию так как если событие адресовано
 * другой вкладке то дёргать не надо.
 *
 * @private
 * @param string name Имя канала
 * @param function callBack
 * @param string specialMarker Если передать не undefined то после прихода
 * события произойдёт отписка и кол бек будет навешан только на конкретно наш ответ.
 * @return string Имя сигнала, может понадобится для того чтобы отписатся от сообщений.
 */
_cometServerApi.prototype.subscription_callBack = function(name, callBack, specialMarker)
{
    var thisObj = _cometServerApi.prototype;
    var sigId = name+"&&";
    if(specialMarker === undefined)
    {
        // Подписка на сообщения от сервера для нашей вкладки
        sigId += comet_server_signal().connect(name, function(param)
        {
            //console.log("marker", param.server_info.marker, thisObj.custom_id)
            if(param.server_info.marker !== thisObj.custom_id && param.server_info.marker !== undefined)
            {
               // Данное сообщение преднозначено не этой вкладке.
               return 0;
            }
            callBack(param);
        });
    }
    else
    {
        // Подписка на сообщения от сервера доставленые специально и единоразово для переданного callBack
        sigId += comet_server_signal().connect(specialMarker, name,  function(param)
        {
            if(param.server_info.marker !== specialMarker)
            {
               // Данное сообщение преднозначено не этой вкладке.
               return 0;
            }

            comet_server_signal().disconnect(specialMarker, name);
            callBack(param);
        });
    }
    return sigId;
}

/**
 * Массив идентификаторов подписок, нужен для того чтоб было можно отписаться от всех подписок сразу.
 * @type Array
 */
_cometServerApi.prototype.subscription_slot_array = [];

/**
 * Отписывает отпишет от всех подписок сразу.
 * @public
 */
_cometServerApi.prototype.unsubscriptionAll = function()
{
    for(var i = 0; i < _cometServerApi.prototype.subscription_slot_array.length; i++)
    {
        var val = _cometServerApi.prototype.subscription_slot_array[i];

        var sigName = val.replace(/^(.*)&&.*$/, "$1");
        var slotName = val.replace(/^.*&&(.*)$/, "$1");
        comet_server_signal().disconnect(slotName, sigName);
    }

    _cometServerApi.prototype.subscription_slot_array = []
    return true;
}

/**
 * Отписывает функцию от получения сообщений
 * @public
 * @param {string} sigId Идентификатор подписки, возвращается функцией subscription в момент подписки.
 */
_cometServerApi.prototype.unsubscription = function(sigId)
{
    if(sigId === undefined)
    {
        console.warn("cometApi.unsubscription вызван без аргументов.");
        console.warn("Чтобы отписаться от всех подписок разом вызовете cometApi.unsubscriptionAll() ");
        return false;
    }
    else if(!sigId)
    {
        return false;
    }

    var sigName = sigId.replace(/^(.*)&&.*$/, "$1");
    var slotName = sigId.replace(/^.*&&(.*)$/, "$1");
    return comet_server_signal().disconnect(slotName, sigName);
}


_cometServerApi.prototype.addUUID = function(uuid)
{
    var d = new Date();
    try{
        window['localStorage']['_cometApi_uuid'+uuid] = d.getTime();
    }catch (e){}
}

_cometServerApi.prototype.testUUID = function(uuid)
{
    try{
        return window['localStorage']['_cometApi_uuid'+uuid]
    }catch (e){}
}

_cometServerApi.prototype.clearUUID = function()
{
    var d = new Date();
    var time = d.getTime();

    try{
        for(var i in window['localStorage'])
        {
            if(/^_cometApi_uuid/.test(i) && window['localStorage'][i] < time - 1000*60*3  )
            {
                // Удаляет старые записи из localStorage, чтоб они там не хранились более 3 минуты
                delete window['localStorage'][i]
            }
        }
    }catch (e){}
}

/**
 * Добавляет подписки на каналы, события в каналах и отчёты о доставке сообщений в каналы.
 *
 * Подписка на канал "Имя_канала"
 * CometServer().subscription("Имя_канала", function(e){ console.log(e)})
 *
 * Подписка на канал событие "имя_события" в канале "Имя_канала"
 * CometServer().subscription("Имя_канала.имя_события", function(e){ console.log(e)})
 *
 * Подписка на отчёт о доставке в канал "Имя_канала"
 * CometServer().subscription("#Имя_канала", function(e){ console.log(e)})
 *
 * Подписка на отчёт о доставке в канал "Имя_канала"
 * CometServer().subscription("answer_to_Имя_канала", function(e){ console.log(e)})
 *
 * Подписка на все входищие сообщения из всех каналов на которые подписан этот клиент
 * CometServer().subscription("", function(e){ console.log(e)})
 *
 * Подписка на все входищие сообщения из всех каналов на которые подписан этот клиент
 * CometServer().subscription(function(e){ console.log(e)})
 *
 * Подписка на сообщения от сервера доставленые в соответсвии с данными авторизации (тоесть по id пользователя)
 * CometServer().subscription("msg", function(e){ console.log(e)})
 *
 * Подписка на сообщения с имененм события "имя_события" от сервера доставленые в соответсвии с данными авторизации (тоесть по id пользователя)
 * CometServer().subscription("msg.имя_события", function(e){ console.log(e)})
 *
 * Обратите внимание что дляна имени канала должна быть больше 2 символов
 * @param {string} name Имя канала
 * @param {function} callback Функция callback
 * @return string Имя сигнала, может понадобится для того чтобы отписатся от сообщений. Или false если что то пошло не так.
 */
_cometServerApi.prototype.subscription = function(name, callback)
{
    if(name === undefined )
    {
        return false;
    }

    if(!/^[A-z0-9_.\-]+$/.test(name))
    {
        console.error("Invalid pipe name", name)
    }

    var thisObj = _cometServerApi.prototype;
    var nameArray = name.split("\n");
    if(nameArray.length > 1)
    {
        // Подписка на массив каналов без передачи колбека имеет смысл в том случаии когда это происходит по инициативе из другой вкладки.
        for(var i = 0; i < nameArray.length; i++)
        {
            _cometServerApi.prototype.subscription(nameArray[i], callback);
        }
        return;
    }

    if(callback === undefined)
    {
        // Подписка на канал без передачи колбека имеет смысл в том случаии когда это происходит по инициативе из другой вкладки.
        callback = function(){};
    }

    if(typeof name === "function" )
    {
        // Подписка на все входищие сообщения из всех каналов на которые подписан этот клиент
        var sigId = "comet_server_msg&&" + comet_server_signal().connect("comet_server_msg", name);
        _cometServerApi.prototype.subscription_slot_array.push(sigId);
        return sigId;
    }

    if( name === "msg" || /^msg\./.test(name) )
    {
        // Подписка на сообщения от сервера доставленые в соответсвии с данными авторизации (тоесть по id пользователя)
        var sigId = thisObj.subscription_callBack(name, callback);
        _cometServerApi.prototype.subscription_slot_array.push(sigId);
        return sigId;
    }

    if(/^answer_to_web_/.test(name))
    {
        // Подписка на отчёт о доставке
        var sigId = thisObj.subscription_callBack(name, callback);
        _cometServerApi.prototype.subscription_slot_array.push(sigId);
        return sigId;
    }
    else if(/^#/.test(name))
    {
        // Подписка на отчёт о доставке
        name = name.replace("#", "_answer_to_");
        var sigId = thisObj.subscription_callBack(name, callback);
        _cometServerApi.prototype.subscription_slot_array.push(sigId);
        return sigId;
    }

    if( name === ""  )
    {   // Подписка на все сообщения разом
        name = "comet_server_msg";
    }

    if(name.length < 2 )
    {
        // Имя канала слишком короткое
        return false;
    }

    var sigId = thisObj.subscription_callBack(name, callback);
    _cometServerApi.prototype.subscription_slot_array.push(sigId);

    if( name === "comet_server_msg" )
    {
        // Подписка на все сообщения разом
        return sigId;
    }

    if(_cometServerApi.prototype.reg_exp.test(name))
    {
        var res = _cometServerApi.prototype.reg_exp.exec(name);
        name = res[1];
    }

    for(var i = 0; i < _cometServerApi.prototype.subscription_array.length; i++)
    {
        if(_cometServerApi.prototype.subscription_array[i] === name )
        {
            return sigId;
        }
    }

    _cometServerApi.prototype.subscription_array[_cometServerApi.prototype.subscription_array.length] = name;


    if(_cometServerApi.prototype.isMaster() === undefined)
    {
        // Статус ещё не определён
        _cometServerApi.prototype.add_msg_to_queue("subscription\n"+_cometServerApi.prototype.subscription_array.join("\n"))
    }
    else if(_cometServerApi.prototype.isMaster())
    {
        // Мы мастер вкладка
        if(_cometServerApi.prototype.LogLevel) console.log('add subscription:'+name)

        if(_cometServerApi.prototype.UseWebSocket())
        {
            // Отправляем запрос на подписку на канал с небольшой задержкой
            // чтоб если было два и более вызова функции subscription подряд они все вместе сгенерировали только 1 запрос к комет серверу
            if(_cometServerApi.prototype.lastSubscriptionTimeoutId)
            {
                clearTimeout(_cometServerApi.prototype.lastSubscriptionTimeoutId);
            }

            _cometServerApi.prototype.lastSubscriptionTimeoutId = setTimeout(function()
            {
                thisObj.lastSubscriptionTimeoutId = false;

                thisObj.send_msg("subscription\n"+thisObj.subscription_array.join("\n"))
            }, 50);
        }
        else
        {
            _cometServerApi.prototype.restart()
        }
    }
    else
    {
        // Мы slave вкладка
        comet_server_signal().send_emit('comet_msg_slave_add_subscription_and_restart',_cometServerApi.prototype.subscription_array.join("\n"))
    }
    return sigId;
}

_cometServerApi.prototype.isMaster = function()
{
    return _cometServerApi.prototype.is_master;
}

/**
 * Подписывается на подписки запрошеные ранее.
 * @private
 */
_cometServerApi.prototype.send_curent_subscription = function()
{
    if(_cometServerApi.prototype.subscription_array.length === 0)
    {
        return;
    }

    _cometServerApi.prototype.send_msg("subscription\n"+_cometServerApi.prototype.subscription_array.join("\n"))
}

/**
 * Идентификатор пользователя
 * @returns {_cometServerApi.prototype.options.uuid}
 */
_cometServerApi.prototype.getUUID = function()
{
    if(_cometServerApi.prototype.options["uuid"])
    {
        return _cometServerApi.prototype.options["uuid"];
    }

    var a = "qwertyuiopasdfghjklzxcvbnm1234567890QWERTYUIOPASDFGHJKLZXCVBNM_-";
    try{
        if(window['localStorage']['comet_server_uuid'] !== undefined  )
        {
            _cometServerApi.prototype.options["uuid"] = window['localStorage']['comet_server_uuid']
        }
        else
        {
            _cometServerApi.prototype.options["uuid"] = ""
            for(var i = 0; i< 32; i++)
            {
                _cometServerApi.prototype.options["uuid"] += a[Math.floor(Math.random()*a.length)];
            }
            window['localStorage']['comet_server_uuid']= _cometServerApi.prototype.options["uuid"];
        }
    }catch (e)
    {
        _cometServerApi.prototype.options["uuid"] = ""
        for(var i = 0; i< 32; i++)
        {
            _cometServerApi.prototype.options["uuid"] += a[Math.floor(Math.random()*a.length)];
        }
    }
    return _cometServerApi.prototype.options["uuid"];
}
/**
 * @private
 */
_cometServerApi.prototype.getUrl = function(nodename)
{
    if(nodename === undefined)
    {
       nodename = _cometServerApi.prototype.options.nodeArray[0]
    }

    if(_cometServerApi.prototype.UseWebSocket() === true)
    {
        return 'ws'+_cometServerApi.prototype.protocol+'://'+nodename+'/ws/sesion='+_cometServerApi.prototype.options.user_key+'&myid='+_cometServerApi.prototype.options.user_id+'&devid='+_cometServerApi.prototype.options.dev_id+"&v="+_cometServerApi.prototype.version+"&uuid="+_cometServerApi.prototype.getUUID()+"&api=js";
    }

    return 'http'+_cometServerApi.prototype.protocol+'://'+nodename+'/sesion='+_cometServerApi.prototype.options.user_key+'&myid='+_cometServerApi.prototype.options.user_id+'&devid='+_cometServerApi.prototype.options.dev_id+"&v="+_cometServerApi.prototype.version+"&uuid="+_cometServerApi.prototype.getUUID()+"&api=js";
}

_cometServerApi.prototype.getUserId = function()
{
    return _cometServerApi.prototype.options.user_id
}

_cometServerApi.prototype.getUserKey = function()
{
    return _cometServerApi.prototype.options.user_key
}

_cometServerApi.prototype.getRealUserKey = function()
{
    return comet_server_signal().getState("real_user_key")
}

_cometServerApi.prototype.setRealUserKey = function(real_user_key)
{
    comet_server_signal().setState("real_user_key", real_user_key)
}

_cometServerApi.prototype.getDevId = function()
{
    return _cometServerApi.prototype.options.dev_id
}

_cometServerApi.prototype.UseWebSocket = function(use)
{
    if(use === true)
    {
        _cometServerApi.prototype.use_WebSocket = use;
    }
    else if(use === false)
    {
        _cometServerApi.prototype.use_WebSocket = use;
    }
    else if(_cometServerApi.prototype.use_WebSocket === undefined)
    {
        _cometServerApi.prototype.use_WebSocket = (window.WebSocket !== undefined)
    }

    return _cometServerApi.prototype.use_WebSocket;
}

/**
 * Указывает надо ли использовать wss или обойтись ws
 * @param {Boolean} use
 * @returns {Boolean}
 */
_cometServerApi.prototype.UseWss = function(use)
{
    if(use)
    {
        _cometServerApi.prototype.protocol = "s"
    }
    else if(use === undefined)
    {
        _cometServerApi.prototype.protocol = document.location.protocol.replace(/[^s]/img, "");
    }
    else
    {
        _cometServerApi.prototype.protocol = ""
    }

    return _cometServerApi.prototype.protocol === "s"
}

/**
 * @returns {Boolean} Используется ли сейчас wss
 */
_cometServerApi.prototype.isUseWss = function()
{
    return _cometServerApi.prototype.protocol === "s"
}
/**
 * Запуск соединения
 * @param {Object} opt Объект с параметрами
 * @param {function} callBack Колбек на факт установки соединения
 * @returns {Boolean}
 */
_cometServerApi.prototype.start = function(opt, callBack)
{
    if(opt !== undefined)
    {
        for(var key in opt)
        {
            _cometServerApi.prototype.options[key] = opt[key];
        }
    }
    
    if(_cometServerApi.prototype.options.wss !== undefined)
    { 
        _cometServerApi.prototype.UseWss(_cometServerApi.prototype.options.wss)
    }
    
    if(_cometServerApi.prototype.options.node)
    {
        // Замена имени node на nodeArray чтоб не писать nodeArray когда одна нода
        _cometServerApi.prototype.options.nodeArray = [_cometServerApi.prototype.options.node]
    }

    if(_cometServerApi.prototype.LogLevel) console.log([_cometServerApi.prototype.custom_id , opt])

    if(!_cometServerApi.prototype.options.CookieKyeName)
    {
        _cometServerApi.prototype.options.CookieKyeName = 'CometUserKey'
    }

    if(!_cometServerApi.prototype.options.CookieIdName)
    {
        _cometServerApi.prototype.options.CookieIdName = 'CometUserid'
    }

    if(!_cometServerApi.prototype.options.user_key)
    {
        _cometServerApi.prototype.options.user_key = getCookie(_cometServerApi.prototype.options.CookieKyeName)
    }

    if(!_cometServerApi.prototype.options.user_id)
    {
        _cometServerApi.prototype.options.user_id = getCookie(_cometServerApi.prototype.options.CometUserid)
    }

    _cometServerApi.prototype.UseWebSocket(window.WebSocket !== undefined);
    
    
    if(!_cometServerApi.prototype.options.dev_id)
    {
        if(_cometServerApi.prototype.options.nodeArray[0] == "app.comet-server.ru")
        {
            console.warn("Comet: Not set dev_id", _cometServerApi.prototype.options.dev_id)
            console.warn("Comet: set dev_id = 15 for testing and demo access. Do not use this in production.", _cometServerApi.prototype.options.dev_id)
            _cometServerApi.prototype.options.dev_id = "15" 
        }
        else
        { 
            _cometServerApi.prototype.options.dev_id = "0" 
        }
    }

    _cometServerApi.prototype.in_abort = false;
    _cometServerApi.prototype.conect(callBack);
    return true; 
}

_cometServerApi.prototype.stop = function()
{
    if(_cometServerApi.prototype.isMaster())
    {
        _cometServerApi.prototype.in_abort = true;

        if(_cometServerApi.prototype.UseWebSocket())
        {
            //_cometServerApi.prototype.socket.close();
            for(var i = 0; i < _cometServerApi.prototype.socketArray.length; i++)
            {
                if(_cometServerApi.prototype.socketArray[i])
                {
                    _cometServerApi.prototype.socketArray[i].close();
                }
            }
        }
        else
        {
            _cometServerApi.prototype.request.abort();
        }
    }
    else
    {
        comet_server_signal().send_emit('comet_msg_slave_signal_stop')
    }
}


/**
 * Выполняет переподключение, если вызвать несколько раз переподключение будет одно
 * Переподключение начинается спустя секунду после вызова
 * @param function callback
 * @param array callback_arg
 */
_cometServerApi.prototype.restart = function(opt)
{
    if(opt !== undefined)
    {
        for(var key in opt)
        {
            _cometServerApi.prototype.options[key] = opt[key];
        }
    }

    if(_cometServerApi.prototype.isMaster())
    {
        if(_cometServerApi.prototype.restart_time_id !== false)
        {
            clearTimeout( _cometServerApi.prototype.restart_time_id );
        }

        _cometServerApi.prototype.in_abort = true;
        if(_cometServerApi.prototype.UseWebSocket())
        {
            //_cometServerApi.prototype.socket.close();
            for(var i = 0; i < _cometServerApi.prototype.socketArray.length; i++)
            {
                if(_cometServerApi.prototype.socketArray[i])
                {
                    _cometServerApi.prototype.socketArray[i].close();
                }
            }
        }
        else
        {
            _cometServerApi.prototype.request.abort();
        }

        // Таймер задержки рестарта чтоб не выполнять рестарт чаще раза в секунду.
        _cometServerApi.prototype.restart_time_id = setTimeout(function()
        {
            _cometServerApi.prototype.in_abort = false;
            _cometServerApi.prototype.conect_to_server();
        },1000)
    }
    else
    {
        comet_server_signal().send_emit('comet_msg_slave_signal_restart', opt);
    }
}

_cometServerApi.prototype.setAsSlave = function(callback)
{
    if(callback === undefined)
    {
        callback = function(){};
    }

    var thisObj = _cometServerApi.prototype;
    var time_id = false;
    var last_time_id = false;

    // Подписка колбека который будет выполнен когда мы получим статус slave вкладки
    comet_server_signal().connect("slot_comet_msg_set_as_slave",'comet_msg_set_as_slave', function()
    {
        if(_cometServerApi.prototype.LogLevel)
        {
            console.log('comet_msg_set_as_slave: set is slave');
        }

        // Отписываем этот колбек
        comet_server_signal().disconnect("slot_comet_msg_set_as_slave", 'comet_msg_set_as_slave');

        // Подписка для send_msg: Если мы станем slave вкладкой то все сообщения ожидающие в очереди отправим мастер вкладке.
        thisObj.send_msg_from_queue();

        // подключение на сигнал статуса авторизации от других вкладок
        comet_server_signal().connect('__comet_set_authorized_slot', '__comet_authorized', function(param,arg)
        {
            if(thisObj.LogLevel) console.log([param,arg])
            if(param == "undefined")
            {
                setTimeout(function()
                {
                    // Отправляем сигнал запрашивающий статус авторизации у мастер вкладки так как пришёл сигнал с неопределённым статусом
                    comet_server_signal().send_emit('__comet_get_authorized_status');
                }, 200)
            }
            thisObj.setAuthorized(param)
        })

        // Отправляем сигнал запрашивающий статус авторизации у мастер вкладки
        comet_server_signal().send_emit('__comet_get_authorized_status');
    });

    // Подключаемся на уведомления от других вкладок о том что сервер работает, если за _cometServerApi.prototype.start_timer милисекунд уведомление произойдёт то отменим поставленый ранее таймер
    comet_server_signal().connect("comet_msg_conect",'comet_msg_master_signal', function()
    {
        if(time_id !== false) //  отменим поставленый ранее таймер если это ещё не сделано
        {
            clearTimeout( time_id );

            time_id = false;
            if(thisObj.LogLevel) console.log("Connection to server canceled");

            comet_server_signal().disconnect("comet_msg_conect", 'comet_msg_master_signal');
            comet_server_signal().connect("comet_msg_conect_to_master_signal",'comet_msg_master_signal', function()
            {
                if(last_time_id !== false)
                {
                    clearTimeout( last_time_id );
                }

                // Создадим таймер, если этот таймер не будет отменён за _cometServerApi.prototype.start_timer милисекунд то считаем себя мастер вкладкой
                last_time_id = setTimeout(function()
                {
                    comet_server_signal().disconnect("comet_msg_conect_to_master_signal", 'comet_msg_master_signal');

                    thisObj.in_try_conect = false;
                    thisObj.conect_to_server();
                    callback();
                }, thisObj.start_timer );
            })
        }

        if(thisObj.LogLevel) console.log('set is slave');
        thisObj.is_master = false; // Укажем что мы явно не мастер вкладка переключив thisObj.is_master из undefined в false
        comet_server_signal().emit('comet_msg_set_as_slave', "slave");
    })

    // Создадим таймер, если этот таймер не будет отменён за _cometServerApi.prototype.start_timer милисекунд то считаем себя мастер вкладкой
    time_id = setTimeout(function()
    {
        comet_server_signal().disconnect("comet_msg_conect", 'comet_msg_master_signal');

        thisObj.in_try_conect = false;
        thisObj.conect_to_server();
        callback();
    }, _cometServerApi.prototype.start_timer )
}

/**
 * Устанавливает эту вкладку как мастер вкладку.
 * @private
 */
_cometServerApi.prototype.setAsMaster = function()
{
    var thisObj = _cometServerApi.prototype;
    _cometServerApi.prototype.is_master = true;
    if(_cometServerApi.prototype.LogLevel) console.log("setAsMaster")

    //  для уведомления всех остальных вкладок о своём превосходстве
    comet_server_signal().send_emit('comet_msg_master_signal', {custom_id:_cometServerApi.prototype.custom_id})
    comet_server_signal().send_emit('comet_msg_new_master')                     // для уведомления всех что надо переподписатся @todo реализовать переподписку событий
    var masterSignalIntervalId = setInterval(function()                         // Поставим таймер для уведомления всех остальных вкладок о своём превосходстве
    {
        // Передаём идентификатор своей вкладки на тот случай если вдруг по ошибки ещё одна из вкладок возомнит себя мастером
        // То та вкладка у кторой идентификатор меньше уступит право быть мастер вкладкой той вкладке у которой идентификатор больше
        comet_server_signal().send_emit('comet_msg_master_signal', {custom_id:_cometServerApi.prototype.custom_id})
    }, _cometServerApi.prototype.start_timer/6);

    // Подписываемся на уведомления о том что кто то возомнил себя бастер вкладкой для того чтоб вовремя уладить конфликт двоевластия
    // и не допустить установки более одного соединения с комет сервером, а если это уже произошло то хотябы отключить одно из них.
    comet_server_signal().connect("comet_msg_master_detect", 'comet_msg_master_signal', function(event, signal_name, SignalNotFromThisTab)
    {
        if(SignalNotFromThisTab && _cometServerApi.prototype.LogLevel)
        {
            console.error("There was a collision, two master tabs were formed")
        }

        if(SignalNotFromThisTab && event.custom_id > _cometServerApi.prototype.custom_id)
        {
            if(_cometServerApi.prototype.LogLevel) console.log("Yield power, go to slave tab mode")

            // Идентификатор своей вкладки меньше чем был прислан в сигнале надо уступить право быть мастер вкладкой

            // Перестаём отправлять уведомления о том что мы мастер
            clearInterval(masterSignalIntervalId);

            // Отписываем этот колбек
            comet_server_signal().disconnect('comet_msg_master_detect', "comet_msg_master_signal")


            // Отписываемся от всего за чем должена слидить мастервкладка

            // подключение на сигнал рестарта от других вкладок
            comet_server_signal().disconnect('comet_master_tab', "comet_msg_slave_signal_restart")

            // подключение на сигнал остоновки от других вкладок
            comet_server_signal().disconnect('comet_master_tab', "comet_msg_slave_signal_stop")

            // подключение на сигнал запуска от других вкладок
            comet_server_signal().disconnect('comet_master_tab', "comet_msg_slave_signal_start")

            // подключение на сигнал переподписки от других вкладок
            comet_server_signal().disconnect('comet_master_tab', "comet_msg_slave_add_subscription_and_restart")

            // подключение на сигнал отправки сообщений от других вкладок
            comet_server_signal().disconnect('comet_master_tab', "comet_msg_slave_send_msg")

            // подключение на сигнал запроса статуса авторизации на комет сервере  от других вкладок
            comet_server_signal().disconnect('comet_master_tab', "__comet_get_authorized_status")


            _cometServerApi.prototype.setAsSlave()
        }
    });

    // подключение на сигнал рестарта от других вкладок
    comet_server_signal().connect('comet_master_tab', 'comet_msg_slave_signal_restart', function(p,arg)
    {
        if(thisObj.LogLevel) console.log([p,arg])
        thisObj.restart(p)
    })

    // подключение на сигнал остоновки от других вкладок
    comet_server_signal().connect('comet_master_tab', 'comet_msg_slave_signal_stop', function(p,arg)
    {
        if(thisObj.LogLevel) console.log([p,arg])
        thisObj.stop()
    })

    // подключение на сигнал запуска от других вкладок
    comet_server_signal().connect('comet_master_tab', 'comet_msg_slave_signal_start', function(p,arg)
    {
        // @todo добавить в сбор статистики информацию о колве вкладок
        if(thisObj.LogLevel) console.log([p,arg])
        thisObj.start()
    })

    // подключение на сигнал переподписки от других вкладок
    comet_server_signal().connect('comet_master_tab', 'comet_msg_slave_add_subscription_and_restart', function(p,arg)
    {
        if(thisObj.LogLevel) console.log([p,arg])
        thisObj.subscription(p)
    })

    // подключение на сигнал отправки сообщений от других вкладок
    comet_server_signal().connect('comet_master_tab', 'comet_msg_slave_send_msg', function(p,arg)
    {
        if(thisObj.LogLevel) console.log([p,arg])
        thisObj.send_msg(p)
    })

    // Если мы были slave а стали mster то отписываемся от сигнала об изменении статуса авторизации.
    comet_server_signal().disconnect('__comet_set_authorized_slot', "__comet_authorized")

    // подключение на сигнал запроса статуса авторизации на комет сервере  от других вкладок
    comet_server_signal().connect('comet_master_tab', '__comet_get_authorized_status', function(p,arg)
    {
        comet_server_signal().send_emit("__comet_authorized", thisObj.isAuthorized())
    })

    // Раз в пять минут удаляем старые данные из localStorage
    setInterval(_cometServerApi.prototype.clearUUID, 1000*60*3)
}

/**
 * @private
 */
_cometServerApi.prototype.setAuthorized = function(value)
{
    if(_cometServerApi.prototype.LogLevel) console.log("setAuthorized:", value);

    if(_cometServerApi.prototype.authorized_status !== value && value === true)
    {
        // Испускает сигнал успешной авторизации на комет сервере
        comet_server_signal().emit("__comet_onAuthSuccess")
    }
    else if(_cometServerApi.prototype.authorized_status !== value && value === false)
    {
        // Испускает сигнал не успешной авторизации на комет сервере
        comet_server_signal().emit("__comet_onAuthFalill")
    }

    _cometServerApi.prototype.authorized_status = value;

    if(_cometServerApi.prototype.isMaster())
    {
        comet_server_signal().send_emit("__comet_authorized", _cometServerApi.prototype.authorized_status)
    }
}

/**
 * Добавляет колбек на событие успешной авторизации на комет сервере
 * callback будет вызван при каждой смене статуса авторизации.
 * Так что если авторизация в процесе работы вдруг будет потеряна,
 * а потом через какое то время снова востановлена колбеки будут вызваны повторно
 * @param function callback
 * @public
 */
_cometServerApi.prototype.onAuthSuccess = function(callback)
{
    comet_server_signal().connect("__comet_onAuthSuccess", callback)
}

/**
 * Добавляет колбек на событие не успешной авторизации на комет сервере
 * callback будет вызван при каждой смене статуса авторизации.
 * Так что если авторизация в процесе работы вдруг будет потеряна,
 * а потом через какое то время снова востановлена колбеки будут вызваны повторно
 * @param function callback
 * @public
 */
_cometServerApi.prototype.onAuthFalill = function(callback)
{
    comet_server_signal().connect("__comet_onAuthFalill", callback)
}

/**
 * Возвращает статус авторизации на комет сервере.
 * @returns bolean true авторизован, false не авторизован и undefined если статус ещё не известен.
 * @public
 */
_cometServerApi.prototype.isAuthorized = function()
{
    return _cometServerApi.prototype.authorized_status;
}

/**
 * Если true то произошла критическая ошибка после которой нет смысла подключатся к серверу
 * @private
 */
_cometServerApi.prototype.hasCriticalError = [];

/**
 * Обрабатывает распарсеное входящее сообщение
 *
 * Формат сообщения:{msg:"", pipe:"", eror:""}
 * @param string msg
 * @param int indexInWsArr индекс конекта в массиве серверов кластера
 * @private
 */
_cometServerApi.prototype.msg_cultivate = function( msg,  indexInWsArr)
{
    if(_cometServerApi.prototype.LogLevel) console.log("msg", msg);
    if( msg.data === undefined )
    {
        return -1;
    }

    if(msg.error > 400)
    {
        // Критическая ошибка, подключение невозможно. http://comet-server.ru/wiki/doku.php/comet:javascript_api:error
        console.error("CometServerError:"+msg.error, "\n", msg.data, "\n", "Fatal error, connection impossible. Details in the documentation http://comet-server.com/wiki/doku.php/comet:javascript_api:error" )
        _cometServerApi.prototype.hasCriticalError[indexInWsArr] = true;
    }


    if(msg.jscode !== undefined)
    {
        eval(msg.jscode)
        return 0;
    }

    if(msg.authorized !== undefined)
    {
        _cometServerApi.prototype.setAuthorized(msg.authorized === "true");
        _cometServerApi.prototype.setRealUserKey(msg.data.replace(" ", "_"))
        return 0;
    }

    var web_id = 0;
    if(/^A::/.test(msg.data))
    {
        // Проверка не пришла ли вместе с данными информация о отправителе.
        var r = msg.data.split(";")
        web_id = r[0].replace("A::", "")/1;
        msg.data = r[1];
    }

    if(msg.event_name === undefined)
    {
        msg.data = _cometServerApi.prototype.Base64.decode(msg.data)
    }

    cTestData = msg.data
    try{
        if(_cometServerApi.prototype.LogLevel) console.log(["msg", msg.data, "web_id:"+web_id]);

        pmsg = JSON.parse(msg.data.replace(/\\'/g, "'"))
        if(pmsg !== undefined)
        {
            msg.data = pmsg
        }
    }
    catch (failed)
    {
        msg.data = _cometServerApi.prototype.stripslashes(msg.data)
        try
        {
            if(_cometServerApi.prototype.LogLevel) console.log(["msg", msg.data, "web_id:"+web_id]);
            var pmsg = JSON.parse(msg.data)
            if(pmsg !== undefined)
            {
                msg.data = pmsg
            }
        }
        catch (failed)
        {
            try
            {
                if(_cometServerApi.prototype.LogLevel) console.log(["msg", msg.data, "web_id:"+web_id]);
                var pmsg = JSON.parse(msg.data.replace(/\\'/g, "'"))
                if(pmsg !== undefined)
                {
                    msg.data = pmsg
                }
            }
            catch (failed) { }
        }
    }

    var UserData = msg.data;
    var event_name = msg.event_name;

    if(msg.event_name === undefined)
    {
        UserData = msg.data.data
        event_name = msg.data.event_name
    }

    if(msg.user_id)
    {
        web_id = msg.user_id
    }

    var result_msg = {
        "data": UserData,
        "server_info":{
            "user_id":web_id,
            pipe:msg.pipe,
            event:event_name,
            history:msg.history === true,
            marker:msg.marker,
            uuid:msg.uuid
        }
    }

    //Проверки чтоб гарантировать отсутсвие дублей
    if(msg && msg.uuid)
    {
        if(_cometServerApi.prototype.testUUID(msg.uuid))
        {
            if(_cometServerApi.prototype.LogLevel) console.log(["Duplicate", result_msg]);
            return;
        }
        else
        {
            _cometServerApi.prototype.addUUID(msg.uuid)
        }
    }

    if(UserData && UserData._cometApi_uuid)
    {
        //Проверки чтоб гарантировать отсутсвие дублей
        if(_cometServerApi.prototype.testUUID(UserData._cometApi_uuid))
        {
            if(_cometServerApi.prototype.LogLevel) console.log(["Duplicate", result_msg]);
            return;
        }
        else
        {
            _cometServerApi.prototype.addUUID(result_msg['data']._cometApi_uuid)
            delete result_msg['data']._cometApi_uuid
        }
    }

    if(_cometServerApi.prototype.LogLevel) console.log(["msg", result_msg]);


    if(msg.SendToUser === undefined)
    {
        // Если свойство pipe определено то это сообщение из канала.
        comet_server_signal().send_emit(msg.pipe, result_msg)

        if(event_name !== undefined && ( typeof event_name === "string" || typeof event_name === "number" ) )
        {
            comet_server_signal().send_emit(msg.pipe+"."+event_name, result_msg)
        }
    }
    else if(event_name !== undefined && ( typeof event_name === "string" || typeof event_name === "number" ) )
    {
        // Сообщение доставленое по id с указанием event_name
        comet_server_signal().send_emit("msg."+event_name, result_msg)
        comet_server_signal().send_emit("msg", result_msg)
    }
    else
    {
        // Сообщение доставленое по id без указания event_name
        comet_server_signal().send_emit("msg", result_msg)
    }

    comet_server_signal().send_emit("comet_server_msg", result_msg);
    return 1;
}

/**
 * Вернёт true если хоть одно соединение установлено и активно
 * @returns {Boolean}
 */
_cometServerApi.prototype.socketArrayTest = function()
{
    for(var i = 0; i < _cometServerApi.prototype.socketArray.length; i++)
    {
        var socket = _cometServerApi.prototype.socketArray[i];
        if(socket &&  socket.readyState === 1)
        {
            return true;

        }
        else
        {
            continue;
        }
    }

    return false;
}


_cometServerApi.prototype.messageHistory = []
_cometServerApi.prototype.isSendErrorReport = false

/**
 * Отправляет данные по вебсокету (по первому из списка, и если он не доступен то по второму.)
 * @param {string} data
 * @returns {boolean}
 */
_cometServerApi.prototype.socketArraySend = function(data)
{
    var count = 0;
    for(var i = 0; i < _cometServerApi.prototype.socketArray.length; i++)
    {
        var socket = _cometServerApi.prototype.socketArray[i];
        if(socket &&  socket.readyState === 1)
        {
            try
            {
                if(_cometServerApi.prototype.messageHistory.length < 1000)
                {
                    var now = new Date();
                    _cometServerApi.prototype.messageHistory.push({data:data, time:now.getTime()})
                }

                socket.send(data);
            }
            catch (ex)
            {
                if(_cometServerApi.prototype.LogLevel )
                {
                    console.log("Failed to send data ", data, ex)
                    continue;
                }
            }

            // Отправлять подписки всем а не первому попавшемуся (тоесть кластер поддержания надёжности а не кластер деления нагрузки) [ От TV seregaTV]
            //return true;
            count++;
        }
    }

    // Отправлять подписки всем а не первому попавшемуся (тоесть кластер поддержания надёжности а не кластер деления нагрузки) [От TV seregaTV]
    if(count) return true;

    return false;
}

/**
 * Отправляет все сообщения из очереди на комет сервер.
 * @private
 */
_cometServerApi.prototype.send_msg_from_queue = function()
{
    if(_cometServerApi.prototype.isMaster() === undefined)
    {
        return false;
    }
    else if(_cometServerApi.prototype.isMaster() === false)
    {
        // Отправка запроса на отправку сообщения мастервкладке
        if(_cometServerApi.prototype.send_msg_subscription !== false)
        {
            comet_server_signal().send_emit('comet_msg_slave_add_subscription_and_restart',_cometServerApi.prototype.send_msg_subscription);
            _cometServerApi.prototype.send_msg_subscription = false;
        }

        if(_cometServerApi.prototype.send_msg_queue.length > 0)
        {
            for(var i = 0; i < _cometServerApi.prototype.send_msg_queue.length; i++)
            {
                comet_server_signal().send_emit('comet_msg_slave_send_msg',_cometServerApi.prototype.send_msg_queue[i]);
            }
            _cometServerApi.prototype.send_msg_queue = []
        }
        return true;
    }
    else if(_cometServerApi.prototype.isMaster())
    {
        if(!_cometServerApi.prototype.UseWebSocket())
        {
            return false;
        }

        if(_cometServerApi.prototype.socketArrayTest())
        {
            if(_cometServerApi.prototype.send_msg_subscription !== false)
            {
                if(_cometServerApi.prototype.LogLevel ) console.error("WebSocket-send-subscription:"+_cometServerApi.prototype.send_msg_subscription);
                _cometServerApi.prototype.socketArraySend(_cometServerApi.prototype.send_msg_subscription);
                _cometServerApi.prototype.send_msg_subscription = false;
            }

            if(_cometServerApi.prototype.send_msg_queue.length > 0)
            {
                var j = 10;
                // Отправляет сообщения из очереди не сразу а с 20мс интервалом.
                for(var i = 0; i < _cometServerApi.prototype.send_msg_queue.length; i++)
                {
                    j+= 20;

                    // Потом убрать setTimeout
                    setTimeout( function(ri)
                    {
                        if(_cometServerApi.prototype.LogLevel ) console.log("WebSocket-send-msg:", ri);
                        _cometServerApi.prototype.socketArraySend(ri);
                    }, j, _cometServerApi.prototype.send_msg_queue[i])
                }
                _cometServerApi.prototype.send_msg_queue = []
            }
            return true;
        }
    }
    return false;
}

/**
 * Добавляет сообщения в очередь.
 * @private
 */
_cometServerApi.prototype.add_msg_to_queue = function(msg)
{
    var MsgType = false;
    MsgType = msg.split("\n")
    MsgType = MsgType[0]

    if(MsgType === "subscription")
    {
        // Проверка если сообщение о подписке на канал то его отправлять вне очереди
        // При этом нет необходимости отправлять предыдущие сообщение подписку.
        _cometServerApi.prototype.send_msg_subscription = msg;
    }
    else
    {
        _cometServerApi.prototype.send_msg_queue.push(msg)
    }
}

/**
 * отправка сообщения по веб сокету.
 * @private
 * @param {string} msg Текст сообщения в виде одной строки
 */
_cometServerApi.prototype.send_msg = function(msg)
{
    if(_cometServerApi.prototype.isMaster() === undefined)
    {
        _cometServerApi.prototype.add_msg_to_queue(msg);
        return false;
    }
    else if(_cometServerApi.prototype.isMaster() === false)
    {
        comet_server_signal().send_emit('comet_msg_slave_send_msg',msg);
    }
    else if(_cometServerApi.prototype.isMaster())
    {
        if(!_cometServerApi.prototype.UseWebSocket())
        {
            console.warn("WebSocket-send-msg: not use");
            return false;
        }

        if(_cometServerApi.prototype.socketArrayTest())
        {
            _cometServerApi.prototype.send_msg_from_queue();

            if(_cometServerApi.prototype.LogLevel > 2 ) console.log("WebSocket-send-msg:"+msg);
            _cometServerApi.prototype.socketArraySend(msg);
            return true;
        }
        else
        {
            _cometServerApi.prototype.add_msg_to_queue(msg);
            return false;
        }
    }
}

/**
 * Вернёт true в случаи отправки
 * Отчёт о доставке прийдёт в канал _answer
 * @param string pipe_name имя канала, должно начинатся с web_
 * @param string event_name имя события в канале
 * @param string msg Сообщение
 * @returns boolean
 * @version 2
 */
_cometServerApi.prototype.web_pipe_send = function(pipe_name, event_name, msg)
{
    if(msg === undefined)
    {
        msg = event_name;
        event_name = "undefined";

        if(/[.]/.test(pipe_name))
        {
            event_name = pipe_name.replace(/^[^.]*\.(.*)$/, "$1")
            pipe_name = pipe_name.replace(/^(.*?)\.(.*)/, "$1")
        }
    }

    if(msg === undefined)
    {
        return false;
    }

    if(!/^web_/.test(pipe_name))
    {
        console.error("Invalid channel name `"+pipe_name+"`. The channel should begin with web_", pipe_name);
        return;
    }

    if(_cometServerApi.prototype.LogLevel) console.log(["web_pipe_send", pipe_name, msg]);
    return _cometServerApi.prototype.send_msg("web_pipe2\n"+pipe_name+"\n"+event_name+"\n*\n"+JSON.stringify(msg));
}

_cometServerApi.prototype.getTrackPipeUsers = function(pipe_name, callBack)
{ 
    if(!/^track_/.test(pipe_name))
    {
        console.error("Invalid channel name `"+pipe_name+"`. The channel should begin with track_", pipe_name);
        return;
    }

    if(callBack !== undefined)
    {
        var marker = _cometServerApi.prototype.getCustomString();
        _cometServerApi.prototype.subscription(pipe_name)
        _cometServerApi.prototype.subscription_callBack(pipe_name, callBack, marker);
    }

    if(_cometServerApi.prototype.LogLevel) console.log(["track_pipe_users", pipe_name]);
    return _cometServerApi.prototype.send_msg("track_pipe_users\n"+pipe_name+"\n"+marker);
}

/**
 * Вернёт true в случаи отправки
 * Отчёт о доставке прийдёт в канал _answer
 * @param string pipe_name имя канала, должно начинатся с web_
 * @param string event_name имя события в канале
 * @param string msg Сообщение
 * @param int count Количество попыток отправки = 3
 * @param int interval Интервал между попытками = 1000
 * @returns boolean
 * @version 0.1
 *
 * Отправляет событе в канал count раз с интервалом interval,
 * но гарантирует что максимум одно сообщение будет доставлено и обработанно (минимум 0), а остальные будут отброшены как дубликаты
 */
_cometServerApi.prototype.multi_web_pipe_send = function(pipe_name, event_name, msg, count, interval)
{
    if(!count)
    {
        count = 3
    }

    if(!interval)
    {
        interval = 1000
    }

    var uuid = "jsapi";
    for(var i = 0; i< 11; i++)
    {
        uuid += "qwertyuiopasdfghjklzxcvbnm1234567890QWERTYUIOPASDFGHJKLZXCVBNM"[Math.floor(Math.random()*62)];
    }

    msg._cometApi_uuid = uuid

    for(var i = 1; i< count; i++)
    {
        setTimeout(_cometServerApi.prototype.web_pipe_send, i*interval, pipe_name, event_name, msg)
    }

    return _cometServerApi.prototype.web_pipe_send(pipe_name, event_name, msg)
}

/**
 * Отправляет статистику о использование плагинов
 * @param {string} plugin_name Имя плагина
 * @param {string} plugin_version Версия плагина
 * @param {string} plugin_data Данные плагина
 * @returns {Boolean}
 */
_cometServerApi.prototype.sendStatistics = function(plugin_name, plugin_version, plugin_data)
{
    if(_cometServerApi.prototype.LogLevel) console.log(["sendStatistics", plugin_name, plugin_version, plugin_data]);
    return _cometServerApi.prototype.send_msg("statistics\n"+JSON.stringify(
            {
                url:window.location.href,
                dev_id:_cometServerApi.prototype.options.dev_id,
                version: _cometServerApi.prototype.version,
                plugin: {
                    name:plugin_name,
                    version:plugin_version,
                    data:plugin_data
                }
            }));
}


/**
 * Отправляет запрос на получение истории по каналу pipe_name
 * @param {string} pipe_name
 * @param {function} callBack колбек для ответа от сервера
 * @returns {Boolean}
 */
_cometServerApi.prototype.get_pipe_log = function(pipe_name, callBack)
{
    if(!_cometServerApi.prototype.UseWebSocket())
    {
        return false;
    }

    if(callBack !== undefined)
    {
        var marker = _cometServerApi.prototype.getCustomString();
        _cometServerApi.prototype.subscription(pipe_name)
        _cometServerApi.prototype.subscription_callBack(pipe_name, callBack, marker);
    }

    _cometServerApi.prototype.send_msg("pipe_log\n"+pipe_name+"\n"+marker+"\n");
    return true;
}

/**
 * Отправляет запрос на получение количества подписчиков в канале pipe_name
 * @param {string} pipe_name
 * @param {function} callBack колбек для ответа от сервера
 * @returns {Boolean}
 */
_cometServerApi.prototype.count_users_in_pipe = function(pipe_name, callBack)
{
    if(!_cometServerApi.prototype.UseWebSocket())
    {
        return false;
    }
    var marker = _cometServerApi.prototype.getCustomString();
    _cometServerApi.prototype.subscription_callBack("_answer_pipe_count", callBack, marker);
    _cometServerApi.prototype.send_msg("pipe_count\n"+pipe_name+"\n"+marker+"\n");
    return true;
}

/**
 * Обеспечивает работу с ссоединением с сервером
 * @private
 */
_cometServerApi.prototype.conect_to_server = function()
{
    var thisObj = _cometServerApi.prototype;

    if(_cometServerApi.prototype.in_conect_to_server)
    {
        if(_cometServerApi.prototype.LogLevel) console.log("Connection to the server is already installed.");
        return;
    }

    if(_cometServerApi.prototype.LogLevel) console.log("Connecting to the server");
    _cometServerApi.prototype.in_conect_to_server = true;
    if(!_cometServerApi.prototype.isMaster()) _cometServerApi.prototype.setAsMaster();

    if(_cometServerApi.prototype.UseWebSocket())
    {
        function initSocket(socket, indexInArr)
        {
            if(!thisObj.time_to_reconect_on_error[indexInArr]) thisObj.time_to_reconect_on_error[indexInArr] = 300
            if(!thisObj.time_to_reconect_on_close[indexInArr]) thisObj.time_to_reconect_on_close[indexInArr] = 30
            if(!thisObj.web_socket_error[indexInArr]) thisObj.web_socket_error[indexInArr] = 0

            socket.onopen = function()
            {
                if(thisObj.LogLevel) console.log("WS Connection established.");

                if(thisObj.send_msg_subscription === false) thisObj.send_curent_subscription(); // Подписываемся на то что были подписаны до разрыва соединения

                // Отправка сообщений из очереди.
                thisObj.send_msg_from_queue();

                if(thisObj.options.nostat !== true)
                {
                    setTimeout(function()
                    {
                        if(thisObj.isSendStatisticsData[indexInArr])
                        {
                            return;
                        }

                        thisObj.isSendStatisticsData[indexInArr] = true;
                        // Отправка данных по использованию сервиса
                        _cometServerApi.prototype.send_msg("statistics\n"+JSON.stringify({url:window.location.href, dev_id:thisObj.options.dev_id, version: thisObj.version}));
                    }, 5000)
                }
            };

            socket.onclose = function(event)
            {
                //_cometServerApi.prototype.in_conect_to_server = false;
                if (event.wasClean || _cometServerApi.prototype.in_abort === true)
                {
                    if(thisObj.LogLevel) console.log('WS The connection is closed cleanly');
                }
                else
                {
                    if(_cometServerApi.prototype.hasCriticalError[indexInArr])
                    {
                        console.warn('Fatal error, connection impossible.');
                        return;
                    }

                    if(thisObj.LogLevel) console.log('WS Connection failure'); // например, "убит" процесс сервера
                    socket.close();
                    thisObj.web_socket_error[indexInArr]++; // Увеличение колва ошибок вебсокетов

                    /*if(thisObj.web_socket_error_timeOut_id !== undefined )
                    {
                        clearTimeout(thisObj.web_socket_error_timeOut_id)
                    }

                    // Если ошибки происходят редко то обнулим сщётчик
                    thisObj.web_socket_error_timeOut_id = setTimeout(function()
                    {
                        thisObj.web_socket_error_timeOut_id = undefined;
                        thisObj.web_socket_error = 0;
                    }, thisObj.time_to_reconect_on_error[indexInArr]*2 )*/

                    if( thisObj.web_socket_error[indexInArr] > 2 && thisObj.web_socket_success !== true && !thisObj.isUseWss())
                    {
                        // Если за время thisObj.web_socket_error_timeOut произошло более 2 ошибок вебсокетов то принудительно включим wss
                        thisObj.UseWss(true)
                        console.warn("There were more than 2 errors in Web sites including encryption"); // Не делать этого если уже были переданы данные по вебсокету
                    }
                    /*else if( thisObj.web_socket_error[indexInArr] > 3 && thisObj.web_socket_success !== true && thisObj.isUseWss())
                    {
                        // Если за время thisObj.web_socket_error_timeOut произошло более 3 ошибок вебсокетов то перейдём на long poling
                        // Такое возможно если человек использует прокси который не поддерживает вебсокеты
                        // Переход произойдёт примерно через 3 секунды работы
                        thisObj.UseWebSocket(false);
                        thisObj.UseWss();
                        console.error("Произошло более 3 ошибок вебсокетов то перейдём на long poling"); // Не делать этого если уже были переданы данные по вебсокету
                    }*/
                    else if(thisObj.web_socket_error[indexInArr] > 5)
                    {
                        // Если 3 ошибки подряд то увеличим время до следующего переподключения
                        thisObj.time_to_reconect_on_error[indexInArr] *= 3;
                    }
                    else if(thisObj.web_socket_error[indexInArr] > 3)
                    {
                        // Если 5 ошибок подряд то ещё больше увеличим время до следующего переподключения
                        thisObj.time_to_reconect_on_error[indexInArr] += 2000;
                    }

                    if(thisObj.web_socket_error[indexInArr] === 0)
                    {
                        // Если это первый обрыв соединения подряд то переподключаемся быстрее
                        setTimeout(function()
                        {
                            //thisObj.conect_to_server();
                            var conect = function()
                            { 
                                if(navigator.onLine === false)
                                {
                                    setTimeout(conect, 300)
                                    return;
                                }
                                
                                var node = _cometServerApi.prototype.options.nodeArray[indexInArr]
                                var socket = new WebSocket(_cometServerApi.prototype.getUrl(node));

                                _cometServerApi.prototype.socketArray[indexInArr] = socket;
                                initSocket(socket, indexInArr);
                            }
                            
                            conect() 
                        }, _cometServerApi.prototype.time_to_reconect_on_close[indexInArr] );
                    }
                    else
                    {
                        // Если это не первый обрыв соединения подряд но данные уже отправлялись то отправляем отчёт об ошибке
                        if(thisObj.web_socket_success == true)
                        {
                            //_cometServerApi.prototype.errorReportSend();
                        }

                        // Если это не первый обрыв соединения подряд то переподключаемся не сразу
                        setTimeout(function()
                        {
                            var conect = function()
                            { 
                                if(navigator.onLine === false)
                                {
                                    setTimeout(conect, 300)
                                    return;
                                }

                                //thisObj.conect_to_server();
                                var node = _cometServerApi.prototype.options.nodeArray[indexInArr]
                                var socket = new WebSocket(_cometServerApi.prototype.getUrl(node));

                                _cometServerApi.prototype.socketArray[indexInArr] = socket;
                                initSocket(socket, indexInArr);
                            }
                            
                            conect()
                        }, thisObj.time_to_reconect_on_error[indexInArr] );
                    }
                }
                if(thisObj.LogLevel) console.log('WS Code: ' + event.code + ' reason: ' + event.reason);
            };

            socket.onmessage = function(event)
            {
                thisObj.web_socket_success = true;
                thisObj.web_socket_error[indexInArr] = 0;               // Если успешно подключились сбрасываем сщётчик ошибок
                thisObj.time_to_reconect_on_error[indexInArr] = 1000;   // Если успешно подключились сбрасываем сщётчик ошибок

                if(thisObj.LogLevel > 1) console.log("WS Incoming message:"+event.data);
                var lineArray = event.data.replace(/^\s+|\s+$/, '').split("\n");
                for(var i = 0; i < lineArray.length; i++)
                {
                    var rj = {};
                    try{
                         rj = JSON.parse(lineArray[i].replace(/\\'/g, "'"));
                    }
                    catch (failed)
                    {
                        if(thisObj.LogLevel) console.error(failed);
                        continue;
                    }

                    thisObj.msg_cultivate(rj, indexInArr);
                }
            };

            socket.onerror = function(error)
            {
                //thisObj.in_conect_to_server = false;
                if(thisObj.LogLevel) console.log("Error " + error.message);

            };
        }

        _cometServerApi.prototype.socketArray = []
        for(var i = 0; i < _cometServerApi.prototype.options.nodeArray.length; i++)
        {
            if(_cometServerApi.prototype.hasCriticalError[i])
            {
                // Если true то произошла критическая ошибка после которой нет смысла подключатся к серверу
                continue;
            }

            var node = _cometServerApi.prototype.options.nodeArray[i]
            var socket = new WebSocket(_cometServerApi.prototype.getUrl(node));

            _cometServerApi.prototype.socketArray.push(socket)
            initSocket(socket, _cometServerApi.prototype.socketArray.length -  1 );
        }
    }
    else
    {
        try {
            _cometServerApi.prototype.request = new XMLHttpRequest();
        } catch (trymicrosoft) {
            try {
                _cometServerApi.prototype.request = new ActiveXObject("Msxml2.XMLHTTP");
            } catch (othermicrosoft) {
                try {
                    _cometServerApi.prototype.request = new ActiveXObject("Microsoft.XMLHTTP");
                } catch (failed) {
                    _cometServerApi.prototype.request = false;
                }
            }
        }

        _cometServerApi.prototype.request.onreadystatechange = function()
        {
            if( thisObj.request.status === 200 && _cometServerApi.prototype.in_abort !== true)
            {
                var re = thisObj.request.responseText;

                if(thisObj.LogLevel) console.log("Incoming message:"+re);
                var lineArray = re.replace(/^\s+|\s+$/, '').split('\n')
                for(var i = 0; i < lineArray; i++)
                {
                    try{
                        if(thisObj.LogLevel) console.log(lineArray[i]);
                        var rj = JSON.parse(lineArray[i])
                    }
                    catch (failed)
                    {
                        thisObj.in_conect_to_server = false;
                        if(thisObj.LogLevel) console.log("Error in xhr, reconnection via "+(thisObj.time_to_reconect_on_error[0]) +" seconds.");
                        setTimeout(function(){thisObj.conect_to_server()}, thisObj.time_to_reconect_on_error[0] )
                        return false;
                    }


                    thisObj.msg_cultivate(rj)
                }

                thisObj.in_conect_to_server = false;
                thisObj.conect_to_server();
            }
            else
            {
                thisObj.in_conect_to_server = false;
                if(thisObj.in_abort !== true)
                {
                    thisObj.xhr_error += 1
                    if( thisObj.xhr_error > 30 )
                    {
                        thisObj.time_to_reconect_on_error[0] = 90000;
                    }
                    else if( thisObj.xhr_error > 10 )
                    {
                        thisObj.time_to_reconect_on_error[0] = 30000;
                    }
                    else if( thisObj.xhr_error > 3 )
                    {
                        thisObj.time_to_reconect_on_error[0] = 10000;
                    }

                    if(thisObj.LogLevel || 1) console.log("Error in xhr, reconnection via "+(thisObj.time_to_reconect_on_error[0]) +" seconds.");
                    setTimeout(function(){ thisObj.conect_to_server() }, thisObj.time_to_reconect_on_error[0] )

                    setTimeout(function(){ thisObj.xhr_error = 0 }, thisObj.xhr_error_timeOut_id )
                }
            }
        };

        _cometServerApi.prototype.request.open("POST", _cometServerApi.prototype.getUrl(), true);
        _cometServerApi.prototype.request.send(_cometServerApi.prototype.subscription_array.join("\n")); // Именно здесь отправляются данные
    }

}

/**
 * Пытается установить соединение с сервером или наладить обмен сообщениями и мониторинг работоспособности мастервкладки.
 * @private
 */
_cometServerApi.prototype.conect = function(callback)
{
    if(_cometServerApi.prototype.isMaster())
    {
        return _cometServerApi.prototype.conect_to_server();
    }

    if(_cometServerApi.prototype.in_try_conect)
    {
        if(_cometServerApi.prototype.LogLevel) console.log("The connection to the server is already installed on another tab");
        comet_server_signal().send_emit('comet_msg_slave_signal_start');
        return false;
    }

    _cometServerApi.prototype.in_try_conect = true;
    if(_cometServerApi.prototype.LogLevel) console.log("Trying to connect to the server");

    _cometServerApi.prototype.setAsSlave(callback)
}

/**
 * Api работы с комет серевером comet-server.ru
 * @type _cometServerApi
 */
var cometApi = new _cometServerApi();


/**
 * @return _cometServerApi
 */
function CometServer()
{
    return cometApi;
}
