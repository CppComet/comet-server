/**
 * JavaScript API for comet-server.com
 * I will be glad to new orders for something a development.
 *
 * VCersion 4.02 
 *
 *
 * @author Trapenok Victor, Levhav@ya.ru, 89244269357
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

if( !window._tabSignal)
{
    var _tabSignal = function(opt)
    {
        this.slotArray = new Array();
        this.debug = false;

        this.sigId = 0;


        this.tabUUID = undefined;
        this.getTabUUID = function()
        {
            if(!this.tabUUID)
            {
                this.tabUUID = "";
                for(var i = 0; i< 16; i++)
                {
                    this.tabUUID += "qwertyuiopasdfghjklzxcvbnm1234567890QWERTYUIOPASDFGHJKLZXCVBNM"[Math.floor(Math.random()*62)];
                }
            }
            return this.tabUUID;
        };

        this.eventKey = this.getTabUUID();
        if(opt && opt.eventKey)
        {
            this.eventKey = opt.eventKey
        }

        this.setEventKey = function(key)
        {
            this.eventKey = key
        }

        this.getEventKey = function()
        {
            return this.eventKey
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
        this.connect = function(slot_name, signal_name, slot_function)
        {
            if(slot_function === undefined)
            {
                slot_function = signal_name;
                signal_name = slot_name;
                slot_name = "sig" + (this.sigId++)
            }

            if (this.slotArray[signal_name] === undefined)
            {
                this.slotArray[signal_name] = {}
            }
            this.slotArray[signal_name][slot_name] = slot_function;
            if(this.debug) console.log("[js-api] На прослушивание сигнала " + signal_name + " добавлен слот " + slot_name + "", this.slotArray);
            return slot_name;
        };


        /**
         * Отписывает слот slot_name от сигнала signal_name
         */
        this.disconnect = function(slot_name, signal_name)
        {
            if (this.slotArray[signal_name] !== undefined)
            {
                if (this.slotArray[signal_name][slot_name] !== undefined)
                {
                    this.slotArray[signal_name][slot_name] = undefined;
                    return true
                }
            }
            return false
        };

        /**
         * Вызывает слоты подписаные на сигнал signal_name и каждому из них передаёт аруметы signal_name - имя вызвавшего сигнала, и param - объект с параметрами для слота)
         * В добавок ретранслирует сигнал в дочернии iframe если они есть и в родительское окно если оно есть
         * @param signal_name Имя сигнала
         * @param param Параметры переданые слоту при вызове в втором аргументе
         * @param SignalNotFromThisTab Если false то значит это сигнал пришёл из другой вкладки
         */
        this.emit = function(signal_name, param, SignalNotFromThisTab)
        {
            //console.log("[js-api] emit:" + signal_name, param)
            if (this.slotArray[signal_name] === undefined)
            {
                if(this.debug) console.log("[js-api] На сигнал " + signal_name + " нет подписчиков")
            }
            else
            {
                if(this.debug) console.log("[js-api] Сигнал " + signal_name + " подписаны слоты");
                var obj = this.slotArray[signal_name];
                for (var slot in obj)
                {
                    if( obj.hasOwnProperty(slot) &&  obj[slot] !== undefined)
                    {
                        obj[slot](param,signal_name, SignalNotFromThisTab === true)
                    }
                }

            }
        };

        /*
         *  генерация события будут оповещены и соседние вкладки
         *  @eName string - имя события
         *  использование .emit('любое название события', [ Параметры события ])
         */
        this.emitAll = function (signal_name, param)
        {
            this.emit(signal_name, param);

            try{
                if(window['localStorage'] !==undefined  )
                {
                    var curent_custom_id = Math.random()+"_"+Math.random()+"_"+Math.random()+"_"+Math.random()+"_"+Math.random();
                    window['localStorage'][this.eventKey]= JSON.stringify({name:signal_name, custom_id:curent_custom_id, param:param});
                }
                return true
            }catch (e){
                return false
            }
        };

        /**
         * Запись состояния общего для всех вкладок
         * @param {string} name
         * @param {object} value
         * @param {number} minTime минимальный возраст данных меньше которого данные перезаписватся не должны в том случаии если они записанны не этой вкладкой
         */
        this.setState = function(name, value, minTime)
        {
            if(!this._states)
            {
                this._states = {}
            }

            var time = new Date();
            try{
                if(minTime)
                {
                    var value = window.localStorage["tabSignal_"+this.eventKey+name];
                    if(value)
                    {
                        var val = JSON.parse(value);

                        if(val.time + minTime > time.getTime() && val.tabUUID != this.getTabUUID() )
                        {
                            // Возраст данных меньше minTime и они записаны не этой вкладкой, а значит мы их перезаписывать не будем
                            return false
                        }
                    }
                }

                window.localStorage["tabSignal_"+this.eventKey+name] = JSON.stringify({time: time.getTime(), value: value, tabUUID: this.getTabUUID()});
                return true
            }catch (e)
            {
                if(minTime)
                {
                    var value = this._states["tabSignal_"+this.eventKey+name];
                    if(value)
                    {
                        var val = JSON.parse(value);

                        if(val.time + minTime > time.getTime() && val.tabUUID != this.getTabUUID() )
                        {
                            // Возраст данных меньше minTime и они записаны не этой вкладкой, а значит мы их перезаписывать не будем
                            return false
                        }
                    }
                }

                this._states["tabSignal_"+this.eventKey+name] = JSON.stringify({time: time.getTime(), value: value, tabUUID: this.getTabUUID()});
                return true
            }
        };

        /**
         * Обновление с интервалом данных чтоб их не перезаписала другая вкладка
         * @param {type} name
         * @param {type} value
         * @param {type} minTime
         * @return {undefined}
         */
        this.intervalUpdateState = function(name, value, minTime)
        {
            var thisObj = this;
            if(thisObj.setState(name, value, minTime))
            {
                return setInterval(function(){
                    thisObj.setState(name, value, minTime)
                }, minTime/3, name, value, minTime)
            }
            return undefined
        };
        /**
         * Чтение состояния общего для всех вкладок
         * @param {string} name
         * @param {number} maxTime Максимальный возраст данных в милесекундах после чего они считаются не действительными.
         * @return {Window.localStorage}
         */
        this.getState = function(name, maxTime)
        {
            if(!this._states)
            {
                this._states = {}
            }

            try{
                var time = new Date();
                var value = window.localStorage["tabSignal_"+this.eventKey+name];
                if(value)
                {
                    var val = JSON.parse(value);

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
            }catch (e){

                var time = new Date();
                var value = this._states["tabSignal_"+this.eventKey+name];
                if(value)
                {
                    var val = JSON.parse(value);

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
            }
            return undefined
        };

        this.send_emit = this.emitAll; // Для совместимости с прошлой версией.

        var thisTabSignalObj = this;
        this.init = true;
        if( window.addEventListener )
        {
            window.addEventListener('storage', function(e)
            {
                if(e.key && e.key == thisTabSignalObj.eventKey)
                {// !testThis
                    try{
                        var data = JSON.parse(e.newValue);
                        if(data !== undefined && data.name !== undefined  )
                        {
                            if(thisTabSignalObj.debug > 1) console.log( data );
                            thisTabSignalObj.emit( data.name, data.param, true )
                        }
                    }
                    catch (failed)
                    {
                    }
                }
            }, false);
        }
        else if( document.attachEvent )
        {
            document.attachEvent('onstorage', function(e)
            {
                if(e.key && e.key == thisTabSignalObj.eventKey)
                {// !testThis
                    try{
                        var data = JSON.parse(e.newValue);
                        if(data !== undefined && data.name !== undefined  )
                        {
                            if(thisTabSignalObj.debug > 1) console.log( data );
                            thisTabSignalObj.emit( data.name, data.param, true )
                        }
                    }
                    catch (failed)
                    {
                    }
                }
            });
        }
    }

}

if( !window.tabSignal)
{
    window.tabSignal = new _tabSignal({eventKey:'tabSignal_storage_emit'});
}

window.comet_server_signal = function(){ return window.tabSignal}

var _cometServerApi = function(opt)
{
    if(opt)
    {
        if(this.options === undefined)
        {
            this.options = {};
        }

        for(var key in opt)
        {
            this.options[key] = opt[key];
        }
    }

    /**
     * @private
     */
    this.version = "4.02";

    /**
     * @private
     */
    this.options = {};

    this.tabSignal = new _tabSignal();

    /**
     * @private
     */
    this.options.nodeArray = ["app.comet-server.ru"];
    this.options.node = undefined;

    /**
     * Режим кластеризации
     * @type Boolean
     *
     * True - подключаемся к одной ноде, если не авторизованы то к случайной, если авторизованы то выбираем на основе user_id
     * False - подключаемся ко всем нодам из списка, отправляем сообщения на одну ноду из списка
     */
    this.options.roundrobin = false;

    /**
     * @private
     */
    this.is_master = undefined;


    /**
     * Префикс для обмена данными в рамках localstorage
     * @private
     */
    this.instance_id = undefined;

    /**
     * @private
     */
    this.in_conect_to_server = false;

    /**
     * @private
     */
    this.in_try_conect = false;

    /**
     * Массив имён каналов на которые мы подписаны
     * @private
     */
    this.subscription_array = new Array();

    /**
     * Случайный идентификатор вкладки.
     * Используется для определения кому предназначены исторические данные из канала.
     * @private
     */
    this.custom_id = (Math.random()*10)+""+Math.random();
    this.custom_id = this.custom_id.replace(/[^0-9A-z]/,"").replace(/^(.{10}).*$/,"$1");


    /**
     * Время на переподключение в милисекундах после второй подряд ошибки подключения
     * @private
     */
    this.time_to_reconect_on_error = [];

    /**
     * Время на переподключение в милисекундах после первой ошибки подключения
     * @private
     */
    this.time_to_reconect_on_close = [];

    /**
     * @private
     */
    this.in_abort = false;

    /**
     * @private
     */
    this.restart_time_id = false;

    /**
     * Время даваемое на определение того какая из вкладок является мастервкладкой
     * @private
     */
    this.start_timer = 1200;

    /**
     * Выражение отделяющие по знаку точки на павую и левую части.
     * @private
     */
    this.reg_exp = new RegExp(/^([^.]+)\.([^.]+)$/);

    /**
     * Определяет надо ли использовать https или http
     * @private
     */
    this.protocol = "";
    if(document.location)
    {
        this.protocol = window.location.protocol.replace(/[^s]/img, "")
    }

    /**
     * @private
     */
    this.web_socket_error = [];

    /**
     * @private
     */
    this.isSendStatisticsData = [];
    /**
     * Учитывает удачно переданные сообщения по вебскокету
     * Если они были то в случаии неполадок с ссетью переход на long poling осуществлён не будет.
     * @private
     */
    this.web_socket_success = false;

    /**
     * @private
     */
    this.web_socket_error_timeOut = 30000;

    /**
     * @private
     */
    this.xhr_error = 0;
    /**
     * @private
     */
    this.xhr_error_timeOut_id = 30000;

    /**
     * @private
     */
    this.authorized_status;

    /**
     * @private
     */
    this.socket;
    this.socketArray = [];

    /**
     * @private
     */
    this.use_WebSocket;

    /**
     * @private
     */
    this.request;

    /**
     * @private
     */
    this.status;

    /**
     * @private
     */
    this.send_msg_queue = [];

    /**
     * содержит пакет данных о подписках готовый к отправке по вебсокету
     * @private
     * @type {string}
     */
    this.send_msg_subscription = false;

    /**
     * Уровень логирования
     * @private
     */
    this.LogLevel = 0;

    /**
     * Список уникальных идентификаторов сообщений которые были приняты
     * Используется чтоб гарантировать отсутсвие дублей
     * @type object
     */
    this.msgsUUIDs = {};

    try{
        if(window['localStorage']['comet_LogLevel'])
        {
            this.LogLevel = window['localStorage']['comet_LogLevel']/1
        }
    }catch (e){}

    this.getLogLevel = function()
    {
        return this.LogLevel;
    };

    this.setLogLevel = function(level)
    {
        this.LogLevel = level;
        try{
            window['localStorage']['comet_LogLevel'] = level;
        }catch (e){}
    };

    /**
     * @return {String} Сдучайная строка ровно из 10 символов
     */
    this.getCustomString = function()
    {
        var custom = (Math.random()*10)+""+Math.random();
        return custom.replace(/[^0-9A-z]/,"").replace(/^(.{10}).*$/,"$1");
    };

    /**
     * @return {string} Уникальный ( с большой долей вероятности ) идентификатор вкладки
     */
    this.getTabUUID = function()
    {
        return this.tabSignal.getTabUUID()
    };

    /**
     *  http://www.webtoolkit.info/
     **/
    this.Base64 = {
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
    };

    this.stripslashes = function(str)
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
    };
    /**
     * Выполняет привязку callBack функции к событию.
     * И при происшествии события на которое мы подписывались в функции subscription
     * определяет надо ли дёргать callBack функцию так как если событие адресовано
     * другой вкладке то дёргать не надо.
     *
     * @private
     * @param {string} name Имя канала
     * @param {function} callBack
     * @param {string} specialMarker Если передать не undefined то после прихода
     * события произойдёт отписка и кол бек будет навешан только на конкретно наш ответ.
     * @return string Имя сигнала, может понадобится для того чтобы отписатся от сообщений.
     */
    this.subscription_callBack = function(name, callBack, specialMarker)
    {
        var thisObj = this;
        var sigId = name+"&&";
        if(specialMarker === undefined)
        {
            // Подписка на сообщения от сервера для нашей вкладки
            sigId += thisObj.tabSignal.connect(name, function(param)
            {
                //console.log("[js-api] marker", param.server_info.marker, thisObj.custom_id)
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
            sigId += thisObj.tabSignal.connect(specialMarker, name,  function(param)
            {
                if(param.server_info.marker !== specialMarker)
                {
                    // Данное сообщение преднозначено не этой вкладке.
                    return 0;
                }

                thisObj.tabSignal.disconnect(specialMarker, name);
                callBack(param);
            });
        }
        return sigId;
    };

    /**
     * Выполняет привязку callBack функции к событию по уникальному маркеру после прихода
     * события произойдёт отписка и кол бек будет навешан только на конкретно наш ответ.
     *
     * @private
     * @param {string} specialMarker
     * @param {function} callBack
     * @return string Имя сигнала, может понадобится для того чтобы отписатся от сообщений.
     */
    this.subscription_once = function(specialMarker, callBack)
    {
        var thisObj = this;
        var sigId = specialMarker+"&&";

        // Подписка на сообщения от сервера доставленые специально и единоразово для переданного callBack
        sigId += thisObj.tabSignal.connect(specialMarker, specialMarker, function(param)
        {
            thisObj.tabSignal.disconnect(specialMarker, specialMarker);
            callBack(param);
        });

        return sigId;
    };

    /**
     * Массив идентификаторов подписок, нужен для того чтоб было можно отписаться от всех подписок сразу.
     * @type Array
     */
    this.subscription_slot_array = [];

    /**
     * Отписывает отпишет от всех подписок сразу.
     * @public
     */
    this.unsubscriptionAll = function()
    {
        for(var i = 0; i < this.subscription_slot_array.length; i++)
        {
            var val = this.subscription_slot_array[i];

            var sigName = val.replace(/^(.*)&&.*$/, "$1");
            var slotName = val.replace(/^.*&&(.*)$/, "$1");
            this.tabSignal.disconnect(slotName, sigName);
        }

        this.subscription_slot_array = [];
        return true;
    };

    /**
     * Отписывает функцию от получения сообщений
     * @public
     * @param {string} sigId Идентификатор подписки, возвращается функцией subscription в момент подписки.
     */
    this.unsubscription = function(sigId)
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
        return this.tabSignal.disconnect(slotName, sigName);
    };


    this.addUUID = function(uuid)
    {
        var d = new Date();
        try{
            window['localStorage']['_cometApi_uuid'+uuid] = d.getTime();
        }catch (e){}
    };

    this.testUUID = function(uuid)
    {
        try{
            return window['localStorage']['_cometApi_uuid'+uuid]
        }catch (e){}
    };

    this.clearUUID = function()
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
    };

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
    this.subscription = function(name, callback)
    {
        if(name === undefined )
        {
            return false;
        }

        var thisObj = this;
        var nameArray = name.split("\n");
        if(nameArray.length > 1)
        {
            // Подписка на массив каналов без передачи колбека имеет смысл в том случаии когда это происходит по инициативе из другой вкладки.
            for(var i = 0; i < nameArray.length; i++)
            {
                this.subscription(nameArray[i], callback);
            }
            return;
        }

        if(callback === undefined)
        {
            // Подписка на канал без передачи колбека имеет смысл в том случаии когда это происходит по инициативе из другой вкладки.
            callback = function(){};
        }

        var sigId = null;
        if(typeof name === "function" )
        {
            // Подписка на все входищие сообщения из всех каналов на которые подписан этот клиент
            sigId = "comet_server_msg&&" + this.tabSignal.connect("comet_server_msg", name);
            this.subscription_slot_array.push(sigId);
            return sigId;
        }

        if( name === "msg" || /^msg\./.test(name) )
        {
            // Подписка на сообщения от сервера доставленые в соответсвии с данными авторизации (тоесть по id пользователя)
            sigId = thisObj.subscription_callBack(name, callback);
            this.subscription_slot_array.push(sigId);
            return sigId;
        }

        if(/^answer_to_web_/.test(name))
        {
            // Подписка на отчёт о доставке
            sigId = thisObj.subscription_callBack(name, callback);
            this.subscription_slot_array.push(sigId);
            return sigId;
        }
        else if(/^#/.test(name))
        {
            // Подписка на отчёт о доставке
            name = name.replace("#", "_answer_to_");
            sigId = thisObj.subscription_callBack(name, callback);
            this.subscription_slot_array.push(sigId);
            return sigId;
        }

        if( name === ""  )
        {   // Подписка на все сообщения разом
            name = "comet_server_msg";
        }

        if(name.length < 2 )
        {
            // Имя канала слишком короткое
            console.error("Error pipe is to short", name);
            return false;
        }

        if(!/^[A-z0-9_.\-]+$/.test(name))
        {
            console.error("Invalid pipe name", name)
        }

        sigId = thisObj.subscription_callBack(name, callback);
        this.subscription_slot_array.push(sigId);

        if( name === "comet_server_msg" )
        {
            // Подписка на все сообщения разом
            return sigId;
        }

        if(this.reg_exp.test(name))
        {
            var res = this.reg_exp.exec(name);
            name = res[1];
        }

        for(var i = 0; i < this.subscription_array.length; i++)
        {
            if(this.subscription_array[i] === name )
            {
                return sigId;
            }
        }

        this.subscription_array[this.subscription_array.length] = name;


        if(this.isMaster() === undefined)
        {
            // Статус ещё не определён
            this.add_msg_to_queue("subscription\n"+this.subscription_array.join("\n"))
        }
        else if(this.isMaster())
        {
            // Мы мастер вкладка
            if(this.LogLevel) console.log("[js-api] add subscription:"+name);

            if(this.UseWebSocket())
            {
                // Отправляем запрос на подписку на канал с небольшой задержкой
                // чтоб если было два и более вызова функции subscription подряд они все вместе сгенерировали только 1 запрос к комет серверу
                if(this.lastSubscriptionTimeoutId)
                {
                    clearTimeout(this.lastSubscriptionTimeoutId);
                }

                this.lastSubscriptionTimeoutId = setTimeout(function()
                {
                    thisObj.lastSubscriptionTimeoutId = false;

                    thisObj.send_msg("subscription\n"+thisObj.subscription_array.join("\n"))
                }, 50);
            }
            else
            {
                this.restart()
            }
        }
        else
        {
            // Мы slave вкладка
            thisObj.tabSignal.emitAll('comet_msg_slave_add_subscription_and_restart',this.subscription_array.join("\n"))
        }
        return sigId;
    };

    this.isMaster = function()
    {
        return this.is_master;
    };

    /**
     * Подписывается на подписки запрошеные ранее.
     * @private
     */
    this.send_curent_subscription = function()
    {
        if(this.subscription_array.length === 0)
        {
            return;
        }

        this.send_msg("subscription\n"+this.subscription_array.join("\n"))
    };

    /**
     * Идентификатор пользователя
     * @return {this.options.uuid}
     */
    this.getUUID = function()
    {
        if(this.options["uuid"])
        {
            return this.options["uuid"];
        }

        var a = "qwertyuiopasdfghjklzxcvbnm1234567890QWERTYUIOPASDFGHJKLZXCVBNM_-";
        try{
            if(window['localStorage']['comet_server_uuid'] !== undefined  )
            {
                this.options["uuid"] = window['localStorage']['comet_server_uuid']
            }
            else
            {
                this.options["uuid"] = "";
                for(var i = 0; i< 32; i++)
                {
                    this.options["uuid"] += a[Math.floor(Math.random()*a.length)];
                }
                window['localStorage']['comet_server_uuid']= this.options["uuid"];
            }
        }catch (e)
        {
            this.options["uuid"] = "";
            for(var i = 0; i< 32; i++)
            {
                this.options["uuid"] += a[Math.floor(Math.random()*a.length)];
            }
        }
        return this.options["uuid"];
    };
    /**
     * @private
     */
    this.getUrl = function(nodename)
    {
        if(nodename === undefined)
        {
            nodename = this.options.nodeArray[0]
        }

        if(this.UseWebSocket() === true)
        {
            return 'ws'+this.protocol+'://'+nodename+'/ws/sesion='+this.options.user_key+'&myid='+this.options.user_id+'&devid='+this.options.dev_id+"&v="+this.version+"&uuid="+this.getUUID()+"&api=js";
        }

        return 'http'+this.protocol+'://'+nodename+'/sesion='+this.options.user_key+'&myid='+this.options.user_id+'&devid='+this.options.dev_id+"&v="+this.version+"&uuid="+this.getUUID()+"&api=js";
    };

    this.getUserId = function()
    {
        return this.options.user_id
    };

    this.getUserKey = function()
    {
        return this.options.user_key
    };

    this.getRealUserKey = function()
    {
        return this.tabSignal.getState("real_user_key")
    };

    this.setRealUserKey = function(real_user_key)
    {
        return this.tabSignal.setState("real_user_key", real_user_key)
    };

    this.getDevId = function()
    {
        return this.options.dev_id
    };

    this.UseWebSocket = function(use)
    {
        if(use === true)
        {
            this.use_WebSocket = use;
        }
        else if(use === false)
        {
            this.use_WebSocket = use;
        }
        else if(this.use_WebSocket === undefined)
        {
            this.use_WebSocket = (window.WebSocket !== undefined)
        }

        return this.use_WebSocket;
    };

    /**
     * Указывает надо ли использовать wss или обойтись ws
     * @param {Boolean} use
     * @return {Boolean}
     */
    this.UseWss = function(use)
    {
        if(use)
        {
            this.protocol = "s"
        }
        else if(use === undefined && window.location && window.location.protocol)
        {
            this.protocol = window.location.protocol.replace(/[^s]/img, "");
        }
        else
        {
            this.protocol = ""
        }

        return this.protocol === "s"
    };

    this.options.isStart = false;

    this.updateEventKey = function()
    {
        this.tabSignal.setEventKey(this.options.nodeArray.join("_")+"_"+this.options.dev_id +"_"+this.options.user_id)
    };

    /**
     * @return {Boolean} Используется ли сейчас wss
     */
    this.isUseWss = function()
    {
        return this.protocol === "s"
    };
    /**
     * Запуск соединения
     * @param {Object} opt Объект с параметрами
     * @param {function} callBack Колбек на факт установки соединения
     * @return {Boolean}
     */
    this.start = function(opt, callBack)
    {
        this.options.isStart = true;

        if(opt !== undefined)
        {
            for(var key in opt)
            {
                this.options[key] = opt[key];
            }
        }

        if(this.options.wss != undefined)
        {
            this.UseWss(this.options.wss)
        }

        if(this.options.node)
        {
            if(typeof this.options.node != "string")
            {
                this.options.nodeArray = this.options.node
            }
            else
            {
                // Замена имени node на nodeArray чтоб не писать nodeArray когда одна нода
                this.options.nodeArray = [this.options.node]
            }
        }

        if(this.options.nodes)
        {
            // Замена имени nodes на nodeArray
            this.options.nodeArray = this.options.nodes
        }

        if(this.LogLevel) console.log("[js-api] start", [this.custom_id , opt]);

        this.UseWebSocket(window.WebSocket !== undefined);


        if(!this.options.dev_id)
        {
            if(this.options.nodeArray[0] == "app.comet-server.ru")
            {
                console.warn("Comet: Not set dev_id", this.options.dev_id);
                console.warn("Comet: set dev_id = 15 for testing and demo access. Do not use this in production.", this.options.dev_id);
                console.warn("Comet: See https://comet-server.com/wiki/doku.php/en:comet:dev_id or https://comet-server.com/wiki/doku.php/comet:dev_id");
                this.options.dev_id = "15"
            }
            else
            {
                this.options.dev_id = "0"
            }
        }

	    this.updateEventKey();

        this.in_abort = false;
        this.conect(callBack);
        return true;
    };

    this.stop = function()
    {
        if(this.isMaster())
        {
            this.in_abort = true;

            if(this.UseWebSocket())
            {
                //this.socket.close();
                for(var i = 0; i < this.socketArray.length; i++)
                {
                    if(this.socketArray[i])
                    {
                        this.socketArray[i].close();
                    }
                }
            }
            else
            {
                this.request.abort();
            }
        }
        else
        {
            this.tabSignal.emitAll('comet_msg_slave_signal_stop')
        }
    };


    /**
     * Выполняет переподключение
     * @param opt опции для переподключения
     * @note не гарантирует правильное переподключение при смене адреса для подключения. Только смена логина и пароля.
     */
    this.restart = function(opt)
    {
        if(opt !== undefined)
        {
            for(var key in opt)
            {
                this.options[key] = opt[key];
            }
        }

	    this.updateEventKey();

        if(!this.options.isStart)
        {
            return this.start(opt);
        }

        if(this.isMaster())
        {
                if(this.UseWebSocket())
                {
                    for(var i = 0; i < this.socketArray.length; i++)
                    {
                        if(this.socketArray[i])
                        {
                            if(this.LogLevel)
                            {
                                console.log("[js-api] restart socket", i);
                            }

                            this.socketArray[i].close();
                        }
                    }
                }
                else
                {
                    this.request.abort();
                }
        }
        else
        {
            this.tabSignal.emitAll('comet_msg_slave_signal_restart', opt);
        }
    };

    this.setAsSlave = function(callback)
    {
        if(callback === undefined)
        {
            callback = function(){};
        }

        var thisObj = this;
        var time_id = false;
        var last_time_id = false;

        // Подписка колбека который будет выполнен когда мы получим статус slave вкладки
        thisObj.tabSignal.connect("slot_comet_msg_set_as_slave",'comet_msg_set_as_slave', function()
        {
            if(thisObj.LogLevel)
            {
                console.log("[js-api] comet_msg_set_as_slave: set is slave");
            }

            // Отписываем этот колбек
            thisObj.tabSignal.disconnect("slot_comet_msg_set_as_slave", 'comet_msg_set_as_slave');

            // Подписка для send_msg: Если мы станем slave вкладкой то все сообщения ожидающие в очереди отправим мастер вкладке.
            thisObj.send_msg_from_queue();

            // подключение на сигнал статуса авторизации от других вкладок
            thisObj.tabSignal.connect('__comet_set_authorized_slot', '__comet_authorized', function(param,arg)
            {
                if(thisObj.LogLevel) console.log([param,arg]);
                if(param == "undefined")
                {
                    setTimeout(function()
                    {
                        // Отправляем сигнал запрашивающий статус авторизации у мастер вкладки так как пришёл сигнал с неопределённым статусом
                        thisObj.tabSignal.emitAll('__comet_get_authorized_status');
                    }, 200)
                }
                thisObj.setAuthorized(param)
            });

            // Отправляем сигнал запрашивающий статус авторизации у мастер вкладки
            thisObj.tabSignal.emitAll('__comet_get_authorized_status');
        });

        // Подключаемся на уведомления от других вкладок о том что сервер работает, если за this.start_timer милисекунд уведомление произойдёт то отменим поставленый ранее таймер
        thisObj.tabSignal.connect("comet_msg_conect",'comet_msg_master_signal', function()
        {
            if(time_id !== false) //  отменим поставленый ранее таймер если это ещё не сделано
            {
                clearTimeout( time_id );

                time_id = false;
                if(thisObj.LogLevel) console.log("[js-api] Connection to server canceled");

                thisObj.tabSignal.disconnect("comet_msg_conect", 'comet_msg_master_signal');
                thisObj.tabSignal.connect("comet_msg_conect_to_master_signal",'comet_msg_master_signal', function()
                {
                    if(last_time_id !== false)
                    {
                        clearTimeout( last_time_id );
                    }

                    // Создадим таймер, если этот таймер не будет отменён за this.start_timer милисекунд то считаем себя мастер вкладкой
                    last_time_id = setTimeout(function()
                    {
                        thisObj.tabSignal.disconnect("comet_msg_conect_to_master_signal", 'comet_msg_master_signal');

                        thisObj.in_try_conect = false;
                        thisObj.conect_to_server();
                        callback();
                    }, thisObj.start_timer );
                })
            }

            if(thisObj.LogLevel) console.log("[js-api] set is slave");
            thisObj.is_master = false; // Укажем что мы явно не мастер вкладка переключив thisObj.is_master из undefined в false
            thisObj.tabSignal.emit('comet_msg_set_as_slave', "slave");
        });

        // Создадим таймер, если этот таймер не будет отменён за this.start_timer милисекунд то считаем себя мастер вкладкой
        time_id = setTimeout(function()
        {
            thisObj.tabSignal.disconnect("comet_msg_conect", 'comet_msg_master_signal');

            thisObj.in_try_conect = false;
            thisObj.conect_to_server();
            callback();
        }, this.start_timer )
    };

    /**
     * Устанавливает эту вкладку как мастер вкладку.
     * @private
     */
    this.setAsMaster = function()
    {
        var thisObj = this;
        this.is_master = true;
        if(this.LogLevel) console.log("[js-api] setAsMaster");

        //  для уведомления всех остальных вкладок о своём превосходстве
        thisObj.tabSignal.emitAll('comet_msg_master_signal', {custom_id:this.custom_id});
        thisObj.tabSignal.emitAll('comet_msg_new_master');                                // для уведомления всех что надо переподписатся @todo реализовать переподписку событий
        var masterSignalIntervalId = setInterval(function()                         // Поставим таймер для уведомления всех остальных вкладок о своём превосходстве
        {
            // Передаём идентификатор своей вкладки на тот случай если вдруг по ошибки ещё одна из вкладок возомнит себя мастером
            // То та вкладка у кторой идентификатор меньше уступит право быть мастер вкладкой той вкладке у которой идентификатор больше
            thisObj.tabSignal.emitAll('comet_msg_master_signal', {custom_id:thisObj.custom_id})
        }, this.start_timer/6);

        // Подписываемся на уведомления о том что кто то возомнил себя бастер вкладкой для того чтоб вовремя уладить конфликт двоевластия
        // и не допустить установки более одного соединения с комет сервером, а если это уже произошло то хотябы отключить одно из них.
        thisObj.tabSignal.connect("comet_msg_master_detect", 'comet_msg_master_signal', function(event, signal_name, SignalNotFromThisTab)
        {
            if(SignalNotFromThisTab && thisObj.LogLevel)
            {
                console.error("There was a collision, two master tabs were formed")
            }

            if(SignalNotFromThisTab && event.custom_id > thisObj.custom_id)
            {
                if(thisObj.LogLevel) console.log("[js-api] Yield power, go to slave tab mode");

                // Идентификатор своей вкладки меньше чем был прислан в сигнале надо уступить право быть мастер вкладкой

                // Перестаём отправлять уведомления о том что мы мастер
                clearInterval(masterSignalIntervalId);

                // Отписываем этот колбек
                thisObj.tabSignal.disconnect('comet_msg_master_detect', "comet_msg_master_signal");


                // Отписываемся от всего за чем должена слидить мастервкладка

                // подключение на сигнал рестарта от других вкладок
                thisObj.tabSignal.disconnect('comet_master_tab', "comet_msg_slave_signal_restart");

                // подключение на сигнал остоновки от других вкладок
                thisObj.tabSignal.disconnect('comet_master_tab', "comet_msg_slave_signal_stop");

                // подключение на сигнал запуска от других вкладок
                thisObj.tabSignal.disconnect('comet_master_tab', "comet_msg_slave_signal_start");

                // подключение на сигнал переподписки от других вкладок
                thisObj.tabSignal.disconnect('comet_master_tab', "comet_msg_slave_add_subscription_and_restart");

                // подключение на сигнал отправки сообщений от других вкладок
                thisObj.tabSignal.disconnect('comet_master_tab', "comet_msg_slave_send_msg");

                // подключение на сигнал запроса статуса авторизации на комет сервере  от других вкладок
                thisObj.tabSignal.disconnect('comet_master_tab', "__comet_get_authorized_status");


                thisObj.setAsSlave()
            }
        });

        // подключение на сигнал рестарта от других вкладок
        thisObj.tabSignal.connect('comet_master_tab', 'comet_msg_slave_signal_restart', function(p,arg)
        {
            if(thisObj.LogLevel) console.log([p,arg]);
            thisObj.restart(p)
        });

        // подключение на сигнал остоновки от других вкладок
        thisObj.tabSignal.connect('comet_master_tab', 'comet_msg_slave_signal_stop', function(p,arg)
        {
            if(thisObj.LogLevel) console.log([p,arg]);
            thisObj.stop()
        });

        // подключение на сигнал запуска от других вкладок
        thisObj.tabSignal.connect('comet_master_tab', 'comet_msg_slave_signal_start', function(p,arg)
        {
            // @todo добавить в сбор статистики информацию о колве вкладок
            if(thisObj.LogLevel) console.log([p,arg]);
            thisObj.start()
        });

        // подключение на сигнал переподписки от других вкладок
        thisObj.tabSignal.connect('comet_master_tab', 'comet_msg_slave_add_subscription_and_restart', function(p,arg)
        {
            if(thisObj.LogLevel) console.log([p,arg]);
            thisObj.subscription(p)
        });

        // подключение на сигнал отправки сообщений от других вкладок
        thisObj.tabSignal.connect('comet_master_tab', 'comet_msg_slave_send_msg', function(p,arg)
        {
            if(thisObj.LogLevel) console.log([p,arg]);
            thisObj.send_msg(p)
        });

        // Если мы были slave а стали mster то отписываемся от сигнала об изменении статуса авторизации.
        thisObj.tabSignal.disconnect('__comet_set_authorized_slot', "__comet_authorized");

        // подключение на сигнал запроса статуса авторизации на комет сервере  от других вкладок
        thisObj.tabSignal.connect('comet_master_tab', '__comet_get_authorized_status', function(p,arg)
        {
            thisObj.tabSignal.emitAll("__comet_authorized", thisObj.isAuthorized())
        });

        // Раз в пять минут удаляем старые данные из localStorage
        setInterval(thisObj.clearUUID, 1000*60*3)
    };

    /**
     * @private
     */
    this.setAuthorized = function(value)
    {
        if(this.LogLevel) console.log("[js-api] setAuthorized:", value);

        if(this.authorized_status !== value && value === true)
        {
            // Испускает сигнал успешной авторизации на комет сервере
            this.tabSignal.emit("__comet_onAuthSuccess")
        }
        else if(this.authorized_status !== value && value === false)
        {
            // Испускает сигнал не успешной авторизации на комет сервере
            this.tabSignal.emit("__comet_onAuthFalill")
        }

        this.authorized_status = value;

        if(this.isMaster())
        {
            this.tabSignal.emitAll("__comet_authorized", this.authorized_status)
        }
    };

    /**
     * Добавляет колбек на событие успешной авторизации на комет сервере
     * callback будет вызван при каждой смене статуса авторизации.
     * Так что если авторизация в процесе работы вдруг будет потеряна,
     * а потом через какое то время снова востановлена колбеки будут вызваны повторно
     * @param {function} callback
     * @public
     */
    this.onAuthSuccess = function(callback)
    {
        this.tabSignal.connect("__comet_onAuthSuccess", callback)
    };

    /**
     * Добавляет колбек на событие не успешной авторизации на комет сервере
     * callback будет вызван при каждой смене статуса авторизации.
     * Так что если авторизация в процесе работы вдруг будет потеряна,
     * а потом через какое то время снова востановлена колбеки будут вызваны повторно
     * @param {function} callback
     * @public
     */
    this.onAuthFalill = function(callback)
    {
        this.tabSignal.connect("__comet_onAuthFalill", callback)
    };

    /**
     * Возвращает статус авторизации на комет сервере.
     * @return bolean true авторизован, false не авторизован и undefined если статус ещё не известен.
     * @public
     */
    this.isAuthorized = function()
    {
        return this.authorized_status;
    };

    /**
     * Если true то произошла критическая ошибка после которой нет смысла подключатся к серверу
     * @private
     */
    this.hasCriticalError = [];

    /**
     * Обрабатывает распарсеное входящее сообщение
     *
     * Формат сообщения:{msg:"", pipe:"", eror:""}
     * @param {string} msg
     * @param {int} indexInWsArr индекс конекта в массиве серверов кластера
     * @private
     */
    this.msg_cultivate = function( msg,  indexInWsArr)
    {
        if(this.LogLevel) console.log("[js-api] msg", msg);
        if( msg.data === undefined )
        {
            return -1;
        }

        if(msg.error > 400)
        {
            // Критическая ошибка, подключение невозможно. http://comet-server.ru/wiki/doku.php/comet:javascript_api:error
            console.error("CometServerError:"+msg.error, "\n", msg.data, "\n", "Fatal error, connection impossible. Details in the documentation http://comet-server.com/wiki/doku.php/comet:javascript_api:error" )
            this.hasCriticalError[indexInWsArr] = true;
        }

        if(msg.jscode !== undefined)
        {
            eval(msg.jscode);
            return 0;
        }

        if(msg.authorized !== undefined && msg.event == "serverInfo" && msg.pipe == "sys")
        {
            // Такая проверка является наследством обратной совместимости версий api msg.authorized === "true" || msg.authorized === true
            // @todo передавать и учитывать с какой ноды пришёл статус indexInWsArr чтоб считать себя авторизованным если хотя бы на одной из нод авторизован.
            this.setAuthorized(msg.authorized === "true" || msg.authorized === true); 
            this.setRealUserKey(msg.data.replace(" ", "_"));
            return 0;
        }

        var web_id = 0;
        if(/^A::/.test(msg.data))
        {
            // Проверка не пришла ли вместе с данными информация о отправителе.
            var r = msg.data.split(";");
            web_id = r[0].replace("A::", "")/1;
            msg.data = r[1];
        }

        //if(msg.event_name === undefined)
        //{
        //    msg.data = this.Base64.decode(msg.data)
        //}

        //console.log("msg.data is", typeof msg.data);

        if(typeof msg.data == "string")
        {
            try{
                //if(this.LogLevel) console.log(["msg", msg.data, "web_id:"+web_id]);

                pmsg = JSON.parse(msg.data.replace(/\\'/g, "'"));
                if(pmsg !== undefined)
                {
                    msg.data = pmsg
                }
            }
            catch (failed)
            {
                msg.data = this.stripslashes(msg.data);
                try
                {
                    //if(this.LogLevel) console.log(["msg", msg.data, "web_id:"+web_id]);
                    var pmsg = JSON.parse(msg.data);
                    if(pmsg !== undefined)
                    {
                        msg.data = pmsg
                    }
                }
                catch (failed)
                {
                    try
                    {
                        //if(this.LogLevel) console.log(["msg", msg.data, "web_id:"+web_id]);
                        var pmsg = JSON.parse(msg.data.replace(/\\'/g, "'"));
                        if(pmsg !== undefined)
                        {
                            msg.data = pmsg
                        }
                    }
                    catch (failed) { 
                    
                        msg.data = this.stripslashes(msg.data);
                        try
                        {
                            //if(this.LogLevel) console.log(["msg", msg.data, "web_id:"+web_id]);
                            var pmsg = JSON.parse(msg.data);
                            if(pmsg !== undefined)
                            {
                                msg.data = pmsg
                            }
                        }
                        catch (failed)
                        {
                            try
                            {
                                //if(this.LogLevel) console.log(["msg", msg.data, "web_id:"+web_id]);
                                var pmsg = JSON.parse(msg.data.replace(/\\'/g, "'"));
                                if(pmsg !== undefined)
                                {
                                    msg.data = pmsg
                                }
                            }
                            catch (failed) { }
                        }
                    }
                }
            }
        }

        //var UserData = msg.data;
        //var event_name = msg.event_name;

        /*if(msg.event_name === undefined)
        {
            UserData = msg.data.data;
            event_name = msg.data.event_name
        }*/

        if(msg.user_id)
        {
            web_id = msg.user_id
        }

        var result_msg = {
            "data": msg.data,
            "server_info":{
                "user_id":web_id,
                pipe:msg.pipe,
                event:msg.event,
                history:msg.history === true,
                marker:msg.marker,
                uuid:msg.uuid
            }
        };

        //Проверки чтоб гарантировать отсутсвие дублей
        if(msg && msg.uuid)
        {
            if(this.testUUID(msg.uuid))
            {
                if(this.LogLevel) console.log(["Duplicate", result_msg]);
                return;
            }
            else
            {
                this.addUUID(msg.uuid)
            }
        }

        if(msg.data && msg.data._cometApi_uuid) // Перепроверить !!!!
        {
            //Проверки чтоб гарантировать отсутсвие дублей
            if(this.testUUID(msg.data._cometApi_uuid)) // Перепроверить !!!!
            {
                if(this.LogLevel) console.log(["Duplicate", result_msg]);
                return;
            }
            else
            {
                this.addUUID(result_msg['data']._cometApi_uuid);
                delete result_msg['data']._cometApi_uuid
            }
        }

        if(this.LogLevel) console.log(["final msg", result_msg]);


        if(msg.pipe != undefined)
        {
            // Если свойство pipe определено то это сообщение из канала.
            this.tabSignal.emitAll(msg.pipe, result_msg);

            if(msg.event !== undefined && ( typeof msg.event === "string" || typeof msg.event === "number" ) )
            {
                this.tabSignal.emitAll(msg.pipe+"."+msg.event, result_msg)
            }
        }
        else if(msg.event !== undefined && ( typeof msg.event === "string" || typeof msg.event === "number" ) )
        {
            // Сообщение доставленое по id с указанием msg.event
            this.tabSignal.emitAll("msg."+msg.event, result_msg);
            this.tabSignal.emitAll("msg", result_msg)
        }
        else
        {
            // Сообщение доставленое по id без указания msg.event
            this.tabSignal.emitAll("msg", result_msg)
        }

        if(msg.marker)
        {
            this.tabSignal.emitAll(msg.marker, result_msg);
        }

        this.tabSignal.emitAll("comet_server_msg", result_msg);
        return 1;
    };

    /**
     * Вернёт true если хоть одно соединение установлено и активно
     * @return {Boolean}
     */
    this.socketArrayTest = function()
    {
        for(var i = 0; i < this.socketArray.length; i++)
        {
            var socket = this.socketArray[i];
            if(socket &&  socket.readyState === 1)
                return true;
        }

        if(this.LogLevel > 3 ) console.log("[js-api] socketArrayTest:false");
        return false;
    };


    this.messageHistory = [];
    this.isSendErrorReport = false;

    /**
     * Отправляет данные по вебсокету (по первому из списка, и если он не доступен то по второму.)
     * @param {string} data
     * @return {boolean}
     */
    this.socketArraySend = function(data)
    {
        var count = 0;
        for(var i = 0; i < this.socketArray.length; i++)
        {
            var socket = this.socketArray[i];
            if(socket &&  socket.readyState === 1)
            {
                try
                {
                    if(this.messageHistory.length < 1000)
                    {
                        var now = new Date();
                        this.messageHistory.push({data:data, time:now.getTime()})
                    }

                    socket.send(data);
                }
                catch (ex)
                {
                    if(this.LogLevel )
                    {
                        console.log("[js-api] Failed to send data ", data, ex);
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
    };

    /**
     * Отправляет все сообщения из очереди на комет сервер.
     * @private
     */
    this.send_msg_from_queue = function()
    {
        var thisObj = this
        if(this.isMaster() === undefined)
        {
            return false;
        }
        else if(this.isMaster() === false)
        {
            // Отправка запроса на отправку сообщения мастервкладке
            if(this.send_msg_subscription !== false)
            {
                this.tabSignal.emitAll('comet_msg_slave_add_subscription_and_restart',this.send_msg_subscription);
                this.send_msg_subscription = false;
            }

            if(this.send_msg_queue.length > 0)
            {
                for(var i = 0; i < this.send_msg_queue.length; i++)
                {
                    this.tabSignal.emitAll('comet_msg_slave_send_msg',this.send_msg_queue[i]);
                }
                this.send_msg_queue = []
            }
            return true;
        }
        else if(this.isMaster())
        {
            if(!this.UseWebSocket())
            {
                return false;
            }

            if(this.socketArrayTest())
            {
                if(this.send_msg_subscription !== false)
                {
                    if(this.LogLevel ) console.error("WebSocket-send-subscription:"+this.send_msg_subscription);
                    this.socketArraySend(this.send_msg_subscription);
                    this.send_msg_subscription = false;
                }

                if(this.send_msg_queue.length > 0)
                {
                    var j = 10;
                    // Отправляет сообщения из очереди не сразу а с 20мс интервалом.
                    for(var i = 0; i < this.send_msg_queue.length; i++)
                    {
                        j+= 20;

                        // Потом убрать setTimeout
                        setTimeout( function(ri)
                        {
                            if(this.LogLevel ) console.log("[js-api] WebSocket-send-msg:", ri);
                            thisObj.socketArraySend(ri);
                        }, j, this.send_msg_queue[i])
                    }
                    this.send_msg_queue = []
                }
                return true;
            }
        }
        return false;
    };

    /**
     * Добавляет сообщения в очередь.
     * @private
     */
    this.add_msg_to_queue = function(msg)
    {
        var MsgType = false;
        MsgType = msg.split("\n");
        MsgType = MsgType[0];

        if(this.LogLevel ) console.log("[js-api] add_msg_to_queue:", msg);
        if(MsgType === "subscription")
        {
            // Проверка если сообщение о подписке на канал то его отправлять вне очереди
            // При этом нет необходимости отправлять предыдущие сообщение подписку.
            this.send_msg_subscription = msg;//.replace(/subscription\n/mg, "");
        }
        else
        {
            this.send_msg_queue.push(msg)
        }
    };

    /**
     * отправка сообщения по веб сокету.
     * @private
     * @param {string} msg Текст сообщения в виде одной строки
     */
    this.send_msg = function(msg)
    {
        if(this.isMaster() === undefined)
        {
            if(this.LogLevel > 3 ) console.log("[js-api] isMaster:undefined");
            this.add_msg_to_queue(msg);
            return false;
        }
        else if(this.isMaster() === false)
        {
            if(this.LogLevel > 3 ) console.log("[js-api] isMaster:false");
            this.tabSignal.emitAll('comet_msg_slave_send_msg',msg);
        }
        else if(this.isMaster())
        {
            if(this.LogLevel > 3 ) console.log("[js-api] isMaster:true");
            if(!this.UseWebSocket())
            {
                console.warn("WebSocket-send-msg: not use");
                return false;
            }

            if(this.socketArrayTest())
            {
                this.send_msg_from_queue();

                if(this.LogLevel > 2 ) console.log("[js-api] WebSocket-send-msg:"+msg);
                this.socketArraySend(msg);
                return true;
            }
            else
            {
                this.add_msg_to_queue(msg);
                return false;
            }
        }
    };

    /**
     * Вернёт true в случаи отправки
     * Отчёт о доставке прийдёт в канал _answer
     * @param {string} pipe_name имя канала, должно начинатся с web_
     * @param {string} event_name имя события в канале
     * @param {string} msg Сообщение
     * @return boolean
     * @version 2
     *
     * @todo добавить в версии 3 колбек на конкретное сообщение.
     */
    this.web_pipe_send = function(pipe_name, event_name, msg)
    {
        if(msg === undefined)
        {
            msg = event_name;
            event_name = "undefined";

            if(/[.]/.test(pipe_name))
            {
                event_name = pipe_name.replace(/^[^.]*\.(.*)$/, "$1");
                pipe_name = pipe_name.replace(/^(.*?)\.(.*)/, "$1");
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

        if(this.LogLevel) console.log(["web_pipe_send", pipe_name, msg]);
        return this.send_msg("web_pipe2\n"+pipe_name+"\n"+event_name+"\n*\n"+JSON.stringify(msg));
    };

    this.getTrackPipeUsers = function(pipe_name, callBack)
    {
        if(!/^track_/.test(pipe_name))
        {
            console.error("Invalid channel name `"+pipe_name+"`. The channel should begin with track_", pipe_name);
            return;
        }

        var marker = this.getCustomString();
        this.subscription_once(marker, callBack);
        /*if(callBack !== undefined)
        {
            this.subscription(pipe_name);
        }*/

        if(this.LogLevel) console.log(["track_pipe_users", pipe_name]);
        return this.send_msg("track_pipe_users\n"+pipe_name+"\n"+marker);
    };

    this.getUserData = function(user_id, callBack)
    {
        if(!user_id || /[^0-9]/.test(user_id))
        {
                console.error("Invalid user_id=`"+user_id+"`. The user_id should is integer");
                return;
        }

        if(callBack === undefined)
            {
                return;
            }

            var marker = this.getCustomString();
            this.subscription_once(marker, callBack);

        if(this.LogLevel) console.log(["user_data", user_id]);
        return this.send_msg("user_data\n"+marker+"\n"+user_id);
    };

    /**
     * Вернёт true в случаи отправки
     * Отчёт о доставке прийдёт в канал _answer
     * @param {string} pipe_name имя канала, должно начинатся с web_
     * @param {string} event_name имя события в канале
     * @param {string} msg Сообщение
     * @param {int} count Количество попыток отправки = 3
     * @param {int} interval Интервал между попытками = 1000
     * @return boolean
     * @version 0.1
     *
     * Отправляет событе в канал count раз с интервалом interval,
     * но гарантирует что максимум одно сообщение будет доставлено и обработанно (минимум 0), а остальные будут отброшены как дубликаты
     */
    this.multi_web_pipe_send = function(pipe_name, event_name, msg, count, interval)
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

        msg._cometApi_uuid = uuid;

        var thisObj = this
        for(var i = 1; i< count; i++)
        {
            setTimeout(function(pipe_name, event_name, msg){
                thisObj.web_pipe_send(pipe_name, event_name, msg)
            }, i*interval, pipe_name, event_name, msg)
        }

        return this.web_pipe_send(pipe_name, event_name, msg)
    };

    /**
     * Отправляет статистику о использование плагинов
     * @param {string} plugin_name Имя плагина
     * @param {string} plugin_version Версия плагина
     * @param {string} plugin_data Данные плагина
     * @return {Boolean}
     */
    this.sendStatistics = function(plugin_name, plugin_version, plugin_data)
    {
        if(this.LogLevel) console.log(["sendStatistics", plugin_name, plugin_version, plugin_data]);
        return this.send_msg("statistics\n"+JSON.stringify(
                {
                    url:window.location.href,
                    dev_id:this.options.dev_id,
                    version: this.version,
                    plugin: {
                        name:plugin_name,
                        version:plugin_version,
                        data:plugin_data
                    }
                }));
    };


    /**
     * Отправляет запрос на получение истории по каналу pipe_name
     * @param {string} pipe_name
     * @param {function} callBack колбек для ответа от сервера
     * @return {Boolean}
     */
    this.get_pipe_log = function(pipe_name, callBack)
    {
        if(!pipe_name)
        {
            console.trace("In CppComet API in get_pipe_log function argument `pipe_name` is required")
            return false;
        }

        if(!this.UseWebSocket())
        {
            return false;
        }

        var marker = this.getCustomString();
        if(callBack !== undefined)
        {
            this.subscription(pipe_name, callBack);

            //marker = this.getCustomString();
            //this.subscription_once(marker, callBack);
        }

        this.send_msg("pipe_log\n"+pipe_name+"\n"+marker+"\n");
        return true;
    };

    /**
     * Отправляет запрос на получение количества подписчиков в канале pipe_name
     * @param {string} pipe_name
     * @param {function} callBack колбек для ответа от сервера
     * @return {Boolean}
     */
    this.count_users_in_pipe = function(pipe_name, callBack)
    {
        if(!this.UseWebSocket())
        {
            return false;
        }
        var marker = this.getCustomString();
        this.subscription_once(marker, callBack);
        this.send_msg("pipe_count\n"+pipe_name+"\n"+marker+"\n");
        return true;
    };

    /**
     * Вернёт false если мы не подключены к серверу
     * @private
     */
    this.isConnected = function()
    {
        for(var i in this.web_socket_error)
        {
            if(this.web_socket_error[i] == 0)
            {
                return true;
            }
        }
        
        return false;
    };
    
    /**
     * Обеспечивает работу с ссоединением с сервером
     * @private
     */
    this.conect_to_server = function()
    {
        var thisObj = this;

        if(this.in_conect_to_server)
        {
            if(this.LogLevel) console.log("[js-api] Connection to the server is already installed.");
            return;
        }

        if(this.LogLevel) console.log("[js-api] Connecting to the server");
        this.in_conect_to_server = true;
        if(!this.isMaster()) this.setAsMaster();

        if(this.UseWebSocket())
        {
            function initSocket(socket, indexInArr)
            {
                if(!thisObj.time_to_reconect_on_error[indexInArr]) thisObj.time_to_reconect_on_error[indexInArr] = 300;
                if(!thisObj.time_to_reconect_on_close[indexInArr]) thisObj.time_to_reconect_on_close[indexInArr] = 30;
                if(!thisObj.web_socket_error[indexInArr]) thisObj.web_socket_error[indexInArr] = 0;

                socket.onopen = function()
                {
                    if(thisObj.LogLevel) console.log("[js-api] WS Connection established.");

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
                            thisObj.send_msg("statistics\n"+JSON.stringify({url:window.location.href, dev_id:thisObj.options.dev_id, version: thisObj.version}));
                        }, 5000)
                    }
                };

                socket.onclose = function(event)
                {
                    //this.in_conect_to_server = false;
                    if (event.wasClean || thisObj.in_abort === true)
                    {
                        if(thisObj.LogLevel) console.log("[js-api] WS The connection is closed cleanly");
                    }
                    else
                    {
                        if(thisObj.hasCriticalError[indexInArr])
                        {
                            console.warn('Fatal error, connection impossible.');
                            return;
                        }

                        if(thisObj.LogLevel) console.log("[js-api] WS Connection failure"); // например, "убит" процесс сервера
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
                            thisObj.UseWss(true);
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
                                        setTimeout(conect, 300);
                                        return;
                                    }

                                    var node = thisObj.options.nodeArray[indexInArr];
                                    var socket = new WebSocket(thisObj.getUrl(node));

                                    thisObj.socketArray[indexInArr] = socket;
                                    initSocket(socket, indexInArr);
                                };

                                conect()
                            }, thisObj.time_to_reconect_on_close[indexInArr] );
                        }
                        else
                        {
                            // Если это не первый обрыв соединения подряд но данные уже отправлялись то отправляем отчёт об ошибке
                            if(thisObj.web_socket_success == true)
                            {
                                //thisObj.errorReportSend();
                            }

                            // Если это не первый обрыв соединения подряд то переподключаемся не сразу
                            setTimeout(function()
                            {
                                var conect = function()
                                {
                                    if(navigator.onLine === false)
                                    {
                                        setTimeout(conect, 300);
                                        return;
                                    }

                                    //thisObj.conect_to_server();
                                    var node = thisObj.options.nodeArray[indexInArr];
                                    var socket = new WebSocket(thisObj.getUrl(node));

                                    thisObj.socketArray[indexInArr] = socket;
                                    initSocket(socket, indexInArr);
                                };

                                conect()
                            }, thisObj.time_to_reconect_on_error[indexInArr] );
                        }
                    }
                    if(thisObj.LogLevel) console.log("[js-api] WS Code: " + event.code + " reason: " + event.reason);
                };

                socket.onmessage = function(event)
                {
                    thisObj.web_socket_success = true;
                    thisObj.web_socket_error[indexInArr] = 0;               // Если успешно подключились сбрасываем сщётчик ошибок
                    thisObj.time_to_reconect_on_error[indexInArr] = 1000;   // Если успешно подключились сбрасываем сщётчик ошибок

                    if(thisObj.LogLevel > 1) console.log("[js-api] \x1b[1;32mWS Incoming message\x1b[0m:"+event.data);
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
                    if(thisObj.LogLevel) console.log("[js-api] Error " + error.message);

                };
            }

            thisObj.socketArray = [];
            var random_node = thisObj.options.user_id % thisObj.options.nodeArray.length;
            if(!random_node)
            {
                random_node = Math.floor(Math.random()*thisObj.options.nodeArray.length)
            }

            for(var i = 0; i < thisObj.options.nodeArray.length; i++)
            {
                if(thisObj.hasCriticalError[i])
                {
                    // Если true то произошла критическая ошибка после которой нет смысла подключатся к серверу
                    continue;
                }


                if(thisObj.options.roundrobin == true && random_node != i)
                {
                    // Если есть опция roundrobin то подключатся будем только к одной ноде на основе своего user_id или случайной ноде если user_id не задан.
                    continue;
                }

                var node = thisObj.options.nodeArray[i];

                console.log("[js-api] conect to " + thisObj.getUrl(node))
                var socket = new window.WebSocket(thisObj.getUrl(node));

                thisObj.socketArray.push(socket);
                initSocket(socket, thisObj.socketArray.length -  1 );
            }
        }
        else
        {
            try {
                thisObj.request = new XMLHttpRequest();
            } catch (trymicrosoft) {
                try {
                    thisObj.request = new ActiveXObject("Msxml2.XMLHTTP");
                } catch (othermicrosoft) {
                    try {
                        thisObj.request = new ActiveXObject("Microsoft.XMLHTTP");
                    } catch (failed) {
                        thisObj.request = false;
                    }
                }
            }

            thisObj.request.onreadystatechange = function()
            {
                if( thisObj.request.status === 200 && thisObj.in_abort !== true)
                {
                    var re = thisObj.request.responseText;

                    if(thisObj.LogLevel) console.log("[js-api] Incoming message:"+re);
                    var lineArray = re.replace(/^\s+|\s+$/, '').split('\n');
                    for(var i = 0; i < lineArray; i++)
                    {
                        try{
                            if(thisObj.LogLevel) console.log(lineArray[i]);
                            var rj = JSON.parse(lineArray[i])
                        }
                        catch (failed)
                        {
                            thisObj.in_conect_to_server = false;
                            if(thisObj.LogLevel) console.log("[js-api] Error in xhr, reconnection via "+(thisObj.time_to_reconect_on_error[0]) +" seconds.");
                            setTimeout(function(){thisObj.conect_to_server()}, thisObj.time_to_reconect_on_error[0] );
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
                        thisObj.xhr_error += 1;
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

                        if(thisObj.LogLevel || 1) console.log("[js-api] Error in xhr, reconnection via "+(thisObj.time_to_reconect_on_error[0]) +" seconds.");
                        setTimeout(function(){ thisObj.conect_to_server() }, thisObj.time_to_reconect_on_error[0] );

                        setTimeout(function(){ thisObj.xhr_error = 0 }, thisObj.xhr_error_timeOut_id )
                    }
                }
            };

            thisObj.request.open("POST", thisObj.getUrl(), true);
            thisObj.request.send(thisObj.subscription_array.join("\n")); // Именно здесь отправляются данные
        }

    };

    /**
     * Пытается установить соединение с сервером или наладить обмен сообщениями и мониторинг работоспособности мастервкладки.
     * @private
     */
    this.conect = function(callback)
    {
        if(this.isMaster())
        {
            return this.conect_to_server();
        }

        if(this.in_try_conect)
        {
            if(this.LogLevel) console.log("[js-api] The connection to the server is already installed on another tab");
            this.tabSignal.emitAll('comet_msg_slave_signal_start');
            return false;
        }

        this.in_try_conect = true;
        if(this.LogLevel) console.log("[js-api] Trying to connect to the server");

        this.setAsSlave(callback)
    };

    return this;
};
var cometServerApi = _cometServerApi

/**
 * Api работы с комет серевером comet-server.ru
 * @type cometServerApi
 */
var cometApi = new cometServerApi();


/**
 * @return _cometServerApi
 */
function CometServer()
{
    return cometApi; //Object.assign({}, _cometServerApi);
}