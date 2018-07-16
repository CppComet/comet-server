/**
 * @author Trapenok Victor (Трапенок Виктор Викторович), Levhav@ya.ru, 89244269357
 * Буду рад новым заказам на разработку чего ни будь.
 *
 * Levhav@ya.ru
 * Skype:Levhav
 * 89244269357
 * @link http://comet-server.com
 * 
 */
  
cometVideoApi = function(){

}

cometVideoApi.version = "1.0"

cometVideoApi.opt = {}

cometVideoApi.opt.language = ["en"]
cometVideoApi.opt.alone_time = 5000

// Качество видео/потребляемый трафик
cometVideoApi.opt.bandwidth_video = 256

// Качество аудио/потребляемый трафик
cometVideoApi.opt.bandwidth_audio = 32

cometVideoApi.useJsSip =  true  

/**
 * Время через которое дозвон прекратится автоматически если не кто не возьмёт трубку
 * @type Number
 */
cometVideoApi.opt.lonelinessTimeout = 1000*60

/**
 * Настройки вызова
 * @type object
 */
cometVideoApi._oConfigCall = { }

/**
 *
 * @param {object} opt
 * @returns {undefined}
 *
 @example
    // Иницируем активацию cometVideoApi
    cometVideoApi.start({

        // Колбек вызываемый перед началом подключения для звонка
        // Предполагается что в нём будут заданы настройки для близжайшего звонка
        // Такие как и параметры audio_remote, video_local, video_remote и возможно ещё какието.
        // А потом будет вызвана функция cometVideoApi.acceptCall(event)
        // А если не будет вызвана то значит мы не взяли трубку.
        onCall:function(callEvent)
        {
            if(!confirm("Ответить на вызов?"))
            {
                // Решили не отвечать на звонок
                return
            }

            // Берём трубку если хотим
            cometVideoApi.acceptCall({
                // Тип звонка 'audio' | 'video'
                type:callType,

                // Указываем целевой элемент для видео потока от собеседника
                video_remote: jQuery("#video_remote")[0],

                // Указываем целевой элемент для аудио потока от собеседника
                audio_remote: jQuery("#audio_remote")[0],

                // Указываем целевой элемент для видео потока от меня (моё собственное изображение из камеры)
                video_local: jQuery("#video_local")[0],
            })
        }
    })
 *  @public
 */
cometVideoApi.start = function(opt)
{
    if(!opt)
    {
        return;
    }

    for(var i in opt)
    {
        cometVideoApi.opt[i] = opt[i]
    }

    if(cometApi.isAuthorized() === undefined)
    {
        cometApi.onAuthSuccess(function()
        {
            // Иницируем активацию cometVideoApi после успешной авторизации на комет сервере.
            // Вероятно этот кусок будет перенесён внутрь файла cometVideoApi
            cometVideoApi._init();
        })
    }
    else if(cometApi.isAuthorized() === true)
    {
        // Иницируем активацию cometVideoApi после успешной авторизации на комет сервере.
        // Вероятно этот кусок будет перенесён внутрь файла cometVideoApi
        cometVideoApi._init();
    }
}

/**
 * Информация о всех входящих звонках на основе их имени комуникационного канала.
 * @type object
 */
cometVideoApi._callPipes = {}

/**
 * Устанавливает Bandwidth видео и аудио
 * Чем больше значение тем выше требования к каналу и тем лучше качество.
 * Устанавливать надо до начала звонка
 * Значение 0 означает значение по умолчанию (как правило это хорошее качество)
 * 
 * @example cometVideoApi.setBandwidth({video:256, audio:32})
 * 
 * @param {object} opt
 * @returns {object} текущее значение качества.
 */
cometVideoApi.setBandwidth = function(opt)
{
    if(opt && opt.video !== undefined)
    {
        cometVideoApi.opt.bandwidth_video = opt.video/1
    }
    
    if(opt && opt.audio !== undefined)
    {
        cometVideoApi.opt.bandwidth_audio = opt.audio/1
    }
    
    return {
        video:cometVideoApi.opt.bandwidth_video,
        audio:cometVideoApi.opt.bandwidth_audio,
    }
}

cometVideoApi.ping = function(pipeName, lonelinessTimeout)
{
    if(cometVideoApi._callPipes[pipeName].data.sys.stream == true
        || cometVideoApi._callPipes[pipeName].data.message.stream == true // @fixme @todo перенести параметр stream в sys часть
        )
    {
        // Так как включён режим stream = true то мы не выходим даже если одни в звонке так как могут быть зрители у стрима. 
        // Так как нам не важно сколько ещё людей то не шлём команду ping
        return;
    }
    
    if(!lonelinessTimeout)
    {
        lonelinessTimeout = cometVideoApi.opt.alone_time
    }

    cometApi.multi_web_pipe_send(pipeName, "ping", { status:"ping"})

    cometVideoApi.pingUpdate(pipeName)
    if(cometVideoApi._callPipes[pipeName].outTimeoutId)
    {
        clearTimeout(cometVideoApi._callPipes[pipeName].outTimeoutId)
        //return;
    }

    // Если кто то вышел то попробуем убедится что кто то кроме нас ещё есть
    // Если за 10 секунд не ответят то и мы выйдем
    cometVideoApi._callPipes[pipeName].outTimeoutId = setTimeout(function()
    {
        console.info("Завершения звонка со статусом loneliness")
        return
        if(cometVideoApi.callEnd(pipeName, {action:"out", status:"loneliness"}))
        {
            cometVideoApi.hangup(cometVideoApi._callPipes[pipeName])
        }
    }, lonelinessTimeout)

}

cometVideoApi.pingCancel  = function(pipeName)
{
    if(!cometVideoApi._callPipes[pipeName])
    {
        return;
    }

    if(cometVideoApi._callPipes[pipeName].outTimeoutId)
    {
        clearTimeout(cometVideoApi._callPipes[pipeName].outTimeoutId)
    }

    if(cometVideoApi._callPipes[pipeName].pingUpdateTimeoutId)
    {
        clearTimeout(cometVideoApi._callPipes[pipeName].pingUpdateTimeoutId)
    }

    cometVideoApi._callPipes[pipeName].pingUpdateTimeoutId = undefined
    cometVideoApi._callPipes[pipeName].outTimeoutId = undefined
}

/**
 * Откладывает на 1 секунду запрос ping к собеседникам
 */
cometVideoApi.pingUpdate = function(pipeName)
{
    if(!cometVideoApi._callPipes[pipeName] || cometVideoApi._callPipes[pipeName].isCall_onCallEnd)
    {
        return false;
    }

    if(cometVideoApi._callPipes[pipeName].pingUpdateTimeoutId)
    {
        clearTimeout(cometVideoApi._callPipes[pipeName].pingUpdateTimeoutId)
    }
    cometVideoApi._callPipes[pipeName].pingUpdateTimeoutId = setTimeout(cometVideoApi.ping, 1000,  pipeName)
}


cometVideoApi._init = function()
{
    if(cometVideoApi.isInit)
    {
        return;
    }
    cometVideoApi.isInit = true;
 
    cometVideoApi.startCallWaiting() 
}

cometVideoApi.isInCall = function()
{
    return cometVideoApi.opt.activeCallEvent !== undefined
}

cometVideoApi.startCallWaiting = function()
{
    // Пришли данные для подключения к sip серверу
    cometApi.subscription("msg.sys_sipCall", function(callEvent)
    {
        console.error("new call", callEvent)
        
        origData =  callEvent.data.message
        
        try{
            // Преобразование строки в JSON если получится
            var res = JSON.parse(callEvent.data.message)
            if(typeof res == "object")
            {
                callEvent.data.message = res
            }
        }catch (exception) {
            try{
                // Преобразование строки в JSON если получится
                var res = JSON.parse(_cometServerApi.prototype.stripslashes(callEvent.data.message))
                if(typeof res == "object")
                {
                    callEvent.data.message = res
                }
            }catch (exception) {

            }
        }
 
        var callPipeName = callEvent.data.sys.callPipe

        if(cometVideoApi._callPipes[callPipeName]
                && cometVideoApi.opt.activeCallEvent
                && cometVideoApi.opt.activeCallEvent.data.sys.callPipe == callPipeName)
        {
            if(!cometVideoApi.opt.activeCallEvent.data.sys.conference) 
            {
                console.log("Входящий звонок в активный диалог", callEvent)
                // Уведомляем что мы заняты так как это входящий звонок в активный диалог а не в конференцию
                cometApi.multi_web_pipe_send(callPipeName, "out", { status:"busy", callKey: callEvent.data.sys.callKey})
                return;
            }


            // Входящий звонок в туже конференцию по которой мы уже говорим
            console.info("Входящий звонок в туже конференцию по которой мы уже говорим", callEvent)

            // Уведомляем что начали взятие трубки
            cometApi.multi_web_pipe_send(callPipeName, "accept", {status:"start_accept"})

            // Ответим что мы тут если кто то сомневается
            cometApi.multi_web_pipe_send(callPipeName, "pong", { status:"pong"})

            return;
        }
        
        console.time("callStart-self")
        console.time("callStart-interlocutor")
        console.time("callStart-ALL")


        cometVideoApi._callPipes[callPipeName] = callEvent


        // Проверка на то что если ни кто не ответит на запрос в течении lonelinessTimeout то прекратить дозвон
        cometVideoApi.ping(callPipeName, cometVideoApi.opt.lonelinessTimeout)


        // Подпишимся на канал сигналинга статусов звонка
        callEvent._callPipeId = cometApi.subscription(callPipeName, function(event)
        {
            var pipeName = event.server_info.pipe
            if(!cometVideoApi._callPipes[pipeName])
            {
                return;
            }

            cometVideoApi.pingUpdate(pipeName)

            if(event.server_info.event == "out")
            {
                console.info("callPipe", event.server_info.event, event)
                if(!cometVideoApi._callPipes[pipeName].data.sys.conference)
                {
                    // Проверка того что в callKey совпадает у нас с тем что пришёл
                    // Проверка callKey поможет сгладить конфликты если мы на разных устройствах авторизовались от одного пользователя. 
                    // Но от злоумышленика имеющего ваш логин и пароль не защитит.
                    if(event.data.callKey == cometVideoApi._callPipes[pipeName].data.sys.callKey)
                    {
                        // Не конференция выходим сразу
                        cometVideoApi.callEnd(pipeName, {
                            action:"out",
                            uuid: event.server_info.uuid,
                            user_id:  event.server_info.user_id,
                            status:event.data.status
                        })
                        cometVideoApi.hangup(cometVideoApi._callPipes[pipeName])
                    }
                    else
                    {
                        console.log("invalid call-key [" + event.data.callKey + "] != [" + cometVideoApi._callPipes[pipeName].data.sys.callKey+"] ")
                    }
                }
                else
                {
                    // Конференция выходим если мы одни в течении cometVideoApi.opt.alone_time милисекунд
                    cometVideoApi.ping(pipeName)

                    if(cometVideoApi.opt.onOut)
                    {
                        cometVideoApi.opt.onOut({
                            action:"out",
                            uuid: event.server_info.uuid,
                            user_id: event.server_info.user_id,
                            status:event.data.status,
                            callInfo: cometVideoApi._callPipes[pipeName].originalEvent.data,
                            type: cometVideoApi._callPipes[pipeName].originalEvent.data.sys.type
                        })
                    }
                }
            }
            else if(event.server_info.event == "toggleMute" && cometVideoApi._callPipes[pipeName].isAccept)
            {
                console.info("callPipe", event.server_info.event, event)
                if(cometVideoApi.opt.onToggleMute)
                {
                    cometVideoApi.opt.toggleMute({
                        action:"toggleMute",
                        uuid: event.server_info.uuid,
                        user_id:  event.server_info.user_id,
                        isMute:event.data.isMute,
                        status:event.data.isMute,
                        callInfo: cometVideoApi._callPipes[pipeName].originalEvent,
                        type: cometVideoApi._callPipes[pipeName].originalEvent.data.sys.type
                    })
                }
            }
            else if(event.server_info.event == "toggleHold" && cometVideoApi._callPipes[pipeName].isAccept)
            {
                console.info("callPipe", event.server_info.event, event)
                if(cometVideoApi.opt.toggleHold)
                {
                    cometVideoApi.opt.toggleHold({
                        action:"toggleHold",
                        uuid: event.server_info.uuid,
                        user_id:  event.server_info.user_id,
                        isHold:event.data.isHold,
                        status:event.data.isHold,
                        callInfo: cometVideoApi._callPipes[pipeName].originalEvent,
                        type: cometVideoApi._callPipes[pipeName].originalEvent.data.sys.type
                    })
                }
            }
            else if(event.server_info.event == "ping"
                    && cometVideoApi._callPipes[pipeName].isAccept
                    && cometVideoApi.opt.activeCallEvent
                    && cometVideoApi.opt.activeCallEvent.data.sys.callPipe == pipeName)
            {
                // Ответим что мы ещё тут если кто то сомневается
                cometApi.multi_web_pipe_send(pipeName, "pong", { status:"pong"})
            }
            else if(event.server_info.event == "pong")
            {
                // Отменим таймер выхода если ещё кто то есть на связи
                if(cometVideoApi._callPipes[pipeName].outTimeoutId)
                {
                    clearTimeout(cometVideoApi._callPipes[pipeName].outTimeoutId)
                    cometVideoApi._callPipes[pipeName].outTimeoutId = undefined
                }
            }
            else if(event.server_info.event == "accept" )
            {
                console.info("callPipe", event.server_info.event, event)
                if(event.server_info.user_id == cometApi.getUserId())
                {
                    console.log("Получили accept от пользователя с нашим id это означает что звонок принят на другом устройстве", event)
                    // выходим сразу если получили accept от пользователя с нашим id
                    // это означает что звонок принят на другом устройстве
                    cometVideoApi.callEnd(pipeName, {
                        action:"cancel",
                        uuid: event.server_info.uuid,
                        user_id:  event.server_info.user_id,
                        status:event.data.status
                    })
                    cometVideoApi.hangup(cometVideoApi._callPipes[pipeName], true)
                    return;
                }

                cometVideoApi.callAccept(pipeName, {
                    uuid: event.server_info.uuid,
                    user_id:  event.server_info.user_id,
                    type: event.data.type,
                })
            }
            else if(event.server_info.event == "mediaStart" )
            {
                console.info("callPipe", event.server_info.event, event)
                cometVideoApi.callStart(pipeName, "interlocutor", event.data.status)
            }
            else if(cometVideoApi._callPipes[pipeName].isAccept)
            {
                console.info("callPipe", event.server_info.event, event)
            }
        })

        cometVideoApi.opt.lastCallEvent = callEvent
        cometVideoApi.opt.lastCallEvent.originalEvent = JSON.parse(JSON.stringify(callEvent))

        cometVideoApi.opt.lastCallEvent._isHangup = false

        if(cometVideoApi.opt.onCall)
        {
            cometVideoApi.opt.onCall({data:cometVideoApi.opt.lastCallEvent.data.message});
        }
        else
        {
            console.error("Не задана функция onCall для обработки входящих звонков");
        }
    })
}

/**
 * Занято, для сброса входящего звонка
 * @public
 * @returns {boolean} True в случаии успеха, иначе False
 */
cometVideoApi.busy = function()
{
    console.info("busy")
    var callPipe = cometVideoApi.opt.lastCallEvent.data.sys.callPipe

    // Запрет вызова callEnd если уже был busy
    cometVideoApi._callPipes[callPipe].isCall_onCallEnd = true
    cometVideoApi.pingCancel(callPipe)

    // Уведомляем что мы заняты
    cometApi.multi_web_pipe_send(callPipe, "out", { status:"busy", callKey:cometVideoApi.opt.lastCallEvent.data.sys.callKey})

    return true
}

/**
 * Вызывает пользовательский колбек завершения звонка
 * @param {string} pipe
 * @param {object} endInfo
 */
cometVideoApi.callEnd = function(pipe, endInfo)
{
    var call = cometVideoApi._callPipes[pipe]
    cometVideoApi.pingCancel(pipe)

    if(!call || call.isCall_onCallEnd)
    {
        return false;
    }

    call.isCall_onCallEnd = true

    var time = new Date()
    call.endTime = time.getTime()

    if(cometVideoApi.opt.onCallEnd)
    {
        endInfo.fullTime = call.endTime - call.startTime  // Полное время от события принятия звонка
        endInfo.time = call.endTime - call.mediaStartTime // Время разговора без учёта времени ожидания подключения
        if( endInfo.status == "busy")
        {
            endInfo.time = 0;
        }
        endInfo.callInfo = call.originalEvent
        endInfo.type = call.originalEvent.data.sys.type
 
        cometVideoApi.opt.onCallEnd({
            action:'out',
            callInfo:endInfo.callInfo.data,
            time:endInfo.time,
            status:endInfo.status,
            fullTime:endInfo.fullTime,
        })
    }

    return true;
}

/**
 * Вызывает пользовательский колбек принятия звонка собеседником (если диалог, а не конференция)
 * @param {string} pipe
 * @param {object} acceptInfo
 */
cometVideoApi.callAccept = function(pipe, acceptInfo)
{
    var call = cometVideoApi._callPipes[pipe]
    if(!call || call.isCall_onCallAccept)
    {
        return false;
    }

    if(call.originalEvent.data.sys.caller_id != cometApi.getUserId())
    {
        // Не вызываем onCallAccept если не мы иницировали звонок
        return false
    }

    call.isCall_onCallAccept = true

    if(cometVideoApi.opt.onCallAccept)
    {
        acceptInfo.action = "accept"
        acceptInfo.status = "accept"
        acceptInfo.callInfo = call.originalEvent

        /*{
            action:"accept",
            uuid: event.server_info.uuid,
            user_id:  event.server_info.user_id,
            isHold:event.data.isHold,
            status:event.data.isHold,
            callInfo: cometVideoApi._callPipes[pipeName].originalEvent
        }*/
        cometVideoApi.opt.onCallAccept({
            action:"accept",
            callInfo:acceptInfo.callInfo.data,
            accept_type:acceptInfo.type
        })
    }

    return true;
}


cometVideoApi.isCallStart = function(pipe)
{
    var call = cometVideoApi._callPipes[pipe]

    if(!call)
    {
        // Что то странное мы не нашли объект разговора
        return false;
    }
    return call.isCall_onCallStart
}
/**
 * Вызывает пользовательский колбек начала разговора
 * @param {string} pipe
 */
cometVideoApi.callStart = function(pipe, person, status)
{
    var call = cometVideoApi._callPipes[pipe]

    if(!call)
    {
        // Что то странное мы не нашли объект разговора
        return false;
    }


    if(person == "interlocutor")
    {
        console.timeEnd("callStart-interlocutor")
        // Собеседник подключился к медиасерверу
        call.call_media_interlocutor_status = true

        if(call.call_media_self_status && status != "answer")
        {
            // Если мы уже подключились к разговору то сообщим об этом тому кто подключается
            cometApi.multi_web_pipe_send(pipe, "mediaStart", { status:"answer"})
        }
    }
    if(person == "self")
    {
        console.timeEnd("callStart-self")
        // Я подключился к медиасерверу
        call.call_media_self_status = true

        // Ответим что мы подключились к разговору
        cometApi.multi_web_pipe_send(pipe, "mediaStart", { status:"self"})
    }

    if(call.isCall_onCallStart)
    {
        // Колбек onCallStart уже вызывался для этого диалога
        return false;
    }

    if( !call.call_media_self_status)
    {
        // Я ещё не завершил свои подключения
        return false;
    }

    if( !call.call_media_interlocutor_status)
    {
        // Cобеседник ещё не завершил свои подключения
        
        if(call.data.sys.stream == true
            || call.data.message.stream == true // @fixme @todo перенести параметр stream в sys часть
            )
        {
            // Так как включён режим stream = true то мы не ждём даже если одни в звонке так как могут быть зрители у стрима.  
        } 
        else
        {
            return false;
        }
    }

    console.timeEnd("callStart-ALL")
    call.isCall_onCallStart = true

    var time = new Date()
    cometVideoApi.opt.activeCallEvent.mediaStartTime = time.getTime()

    if(cometVideoApi.opt.onCallStart)
    {
        console.timeEnd("callStart")

        var info = {}
        info.action = "start"
        info.status = "start"
        info.callInfo = call.originalEvent
        info.type = call.originalEvent.data.sys.type

        // И я и собеседник завершили свои подключения
        cometVideoApi.opt.onCallStart(info)
    }

    return true;
}

/**
 * Взятие трубки для приёма входящего звонка
 * @param {object} opt Взятие трубки для приёма входящего звонка
 * @public
 * @returns {boolean} True в случаии успеха, иначе False
 *
 * Может вернуть False например если на одной из вкладок сайта уже идёт разновор
 *  (Два разговора одновременно могут создавать неприятное эхо)
 *  @todo Проверить этот факт в условии что разговоры с разными людьми а не в одной конфернеции
 *
 *  @example Пример объекта opt для указания настроек звонка
 *  {
        // Тип звонка 'audio' | 'video'
        type:callType,

        // Указываем целевой элемент для видео потока от собеседника
        video_remote: jQuery("#video_remote")[0],

        // Указываем целевой элемент для аудио потока от собеседника
        audio_remote: jQuery("#audio_remote")[0],

        // Указываем целевой элемент для видео потока от меня (моё собственное изображение из камеры)
        video_local: jQuery("#video_local")[0],
    }
 *
 */
cometVideoApi.acceptCall = function(opt)
{
    if(cometVideoApi.opt.activeCallEvent)
    {
        // Наверное допустимо, но тогда надо выключать звук чтоб отправка звука шла только в один звонок.
        console.error("Уже есть один активный звонок. Два звонка одновременно из одного браузера не допустимо.")
        return false;
    }
    // console.log("acceptCall, Начинаем подключение")

    // Последний звонок делаем активным звонком
    cometVideoApi.opt.activeCallEvent = cometVideoApi.opt.lastCallEvent

    var time = new Date()
    cometVideoApi.opt.activeCallEvent.startTime = time.getTime()

    // Отмечаем что звонок принят
    cometVideoApi.opt.activeCallEvent.isAccept = true

    var callPipeName = cometVideoApi.opt.activeCallEvent.data.sys.callPipe

    // Ответим что мы тут если кто то сомневается
    cometApi.multi_web_pipe_send(callPipeName, "pong", { status:"pong"})

    console.log("trackPipe", "track_"+callPipeName)
    // Подпишимся на канал отслеживания списка учасников
    cometVideoApi.opt.activeCallEvent._trackPipeId = cometApi.subscription("track_"+callPipeName,function(event)
    {
        //console.log("trackPipe", event)
        if(!cometVideoApi.opt.activeCallEvent)
        {
            return;
        }

        if(!cometVideoApi.opt.activeCallEvent.data.sys.conference)
        {
            if(event.server_info.event == "subscription")
            {
                // Востановление связи с собеседником
            }
            else if(event.server_info.event == "unsubscription")
            {
                // Потеря связи с собеседником
                // выходим если мы одни в течении cometVideoApi.opt.alone_time милисекунд
                cometVideoApi.ping(callPipeName)
            }
        }
    })


    cometVideoApi.opt.activeCallEvent.call_options = {}
    cometVideoApi.opt.activeCallEvent.call_options.call = {}
    for(var i in opt)
    {
        cometVideoApi.opt.activeCallEvent.call_options[i] = opt[i]
    }

    cometVideoApi._sipUnRegister(function()
    {
        console.log("acceptCall", opt)

        window.URL.createObjectURL = window.URL.createObjectURL || window.URL.webkitCreateObjectURL || window.URL.mozCreateObjectURL || window.URL.msCreateObjectURL;
        var acceptThisCall = function()
        {
            // Уведомляем что начали взятие трубки
            cometApi.multi_web_pipe_send(callPipeName, "accept", {status:"start_accept", type:opt.type})
 
            var userId = "" + cometApi.getUserId();
            var devId = "" + cometApi.getDevId();
            for(var i = devId.length; i < 6; i++)
            {
                devId = "0" + devId
            }

            cometVideoApi._sipRegister({
                realm:cometVideoApi.opt.activeCallEvent.data.sys.serverName,
                impi:devId + userId,
                password:cometApi.getRealUserKey(),
                display_name:devId + userId,
                impu:"sip:"+devId + userId+"@"+cometVideoApi.opt.activeCallEvent.data.sys.serverName,
                websocket_proxy_url:"wss://"+cometVideoApi.opt.activeCallEvent.data.sys.serverName+":"+cometVideoApi.opt.activeCallEvent.data.sys.serverPort
            },
            function()
            {
                if(!cometVideoApi.opt.activeCallEvent)
                {
                    console.log("Не успели взять трубку, звонок завершён")
                    return;
                }
                cometVideoApi._sipCall(opt.type, cometVideoApi.opt.activeCallEvent.data.sys.sipNumber);
            })
        }


        if(cometVideoApi.opt.activeCallEvent.call_options.custom_stream)
        {
            if(opt.video_local)
            {
                // получаем url поточного видео и устанавливаем как источник для video
                opt.video_local.src = window.URL.createObjectURL(cometVideoApi.opt.activeCallEvent.call_options.custom_stream);
            }

            acceptThisCall()
        }
        else
        {
            navigator._getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia;
            
            if(opt.type == 'none')
            {
                // Режим не запрашивать доступ к микрофону и камере.
                cometVideoApi.opt.activeCallEvent.selfStream = stream
                selfStream = stream
                acceptThisCall()
            }
            else
            {
                // запрашиваем разрешение на доступ к поточному видео камеры
                navigator._getUserMedia({audio: true, video: opt.type == 'video'}, function (stream)
                {
                    console.log("Добавили вид себя")
                    if(opt.video_local)
                    {
                        // получаем url поточного видео и устанавливаем как источник для video
                        opt.video_local.src = window.URL.createObjectURL(stream);
                    }

                        cometVideoApi.opt.activeCallEvent.selfStream = stream
                        selfStream = stream
                    acceptThisCall()
                }, function (err)
                {
                    // @todo добавить режим чтоб не отключатчся если доступа нет.
                    
                    console.error('что-то не так с медиастримом или пользователь запретил его использовать', err);
                    if(cometVideoApi.opt.activeCallEvent)
                    {
                        cometVideoApi.callEnd(cometVideoApi.opt.activeCallEvent.data.sys.callPipe, {action:"terminated", status:"no-media-stream", error:err})
                        cometVideoApi.hangup(cometVideoApi.opt.activeCallEvent)
                    }
                });
            }
        }
    })

    return true
}

/**
 * Регистрация на sip сервере, асинхронная
 * sends SIP REGISTER request to login
 *
 * @param {object}  opt
 * @param {callback} callback
 * @private
 */
cometVideoApi._sipRegister = function(regOptions, callback)
{
    console.log("_sipRegister regOptions", regOptions)

    console.info("sip info", {
           password:regOptions.password,
           impu:regOptions.impu,
           realm:regOptions.realm,
           impi:regOptions.impi,
           websocket_url:regOptions.websocket_proxy_url,
           number:cometVideoApi.opt.activeCallEvent.data.sys.sipNumber
        })
 
    var socket = new JsSIP.WebSocketInterface(regOptions.websocket_proxy_url);
    //socket.via_transport = "auto";

    var configuration = {
        sockets  : [ socket ],
        uri      : regOptions.impu,
        password : regOptions.password,
        authorization_user: regOptions.impi,
        registrar_server: "sip:"+regOptions.realm,
        stun_servers: ['stun:stun.l.google.com:19302', 'stun:stun01.sipphone.com', 'stun:stun.ekiga.net', 'stun:stun4.l.google.com:19302'],
        no_answer_timeout: 120,
        display_name: regOptions.impi,
        session_timers: false,
        use_preloaded_route: false,
        realm:regOptions.realm,
        contact_uri: "",
        instance_id: null,
    };

    console.info("configuration", configuration)

    cometVideoApi.coolPhone = new JsSIP.UA(configuration);

    // WebSocket connection events
    cometVideoApi.coolPhone.on('connected', function(e)
    {
        console.info("connected", e)
        tabSignal.emit('_cometVideoApi_connected');
    });

    cometVideoApi.coolPhone.on('disconnected', function(e)
    {
        console.info("connected", e)
        tabSignal.emit('_cometVideoApi_stopped');
    });

    // New incoming or outgoing call event
    cometVideoApi.coolPhone.on('newRTCSession', function(e)
    {
        if (e.direction === 'local')
        {
            console.log('Outgoing call');
        }
        else if (e.direction === 'remote')
        {
            console.log('Incoming call');
            //e.session.answer();
            return;
        }

        cometVideoApi.opt.activeCallEvent.jssipRTCSession = e.session

        /*console.log("isMuted", e.session.isMuted())
        console.log("isOnHold", e.session.isOnHold())

        e.session.on('peerconnection', function(data) {
            console.info("peerconnection", data)
        });
        e.session.on('progress', function(data) {
            console.info("progress", data)
        });
        e.session.on('accepted', function(data) {
            console.info("accepted", data)
        });
        e.session.on('confirmed', function(data) {
            console.info("confirmed", data)
        });
        e.session.on('failed', function(data) {
            console.error("failed", data)
        });
        e.session.on('ended', function(data) {
            console.info("ended", data)
        });


        e.session.on('newDTMF', function(data) {
            console.info("newDTMF", data)
        });
        e.session.on('reinvite', function(data) {
            console.info("reinvite", data)
        });
        e.session.on('update', function(data) {
            console.info("update", data)
        });
        e.session.on('refer', function(data) {
            console.info("refer", data)
        });
        e.session.on('replaces', function(data) {
            console.info("replaces", data)
        });
        e.session.on('sdp', function(data) {
            console.info("sdp", data)
        });
        e.session.on('getusermediafailed', function(data) {
            console.error("getusermediafailed", data)
        });

        peerconnection
        connecting
        sending
        progress
        accepted
        confirmed
        ended
        failed
        newDTMF
        hold
        unhold
        muted
        unmuted
        reinvite
        update
        refer
        replaces
        sdp
        getusermediafailed
        peerconnection:createofferfailed
        peerconnection:createanswerfailed
        peerconnection:setlocaldescriptionfailed
        peerconnection:setremotedescriptionfailed*/

        console.info("newRTCSession", e)
    });

    // New incoming or outgoing IM message event
    cometVideoApi.coolPhone.on('newMessage', function(e)
    {
        console.info("newMessage", e)
    });

    // SIP registration events
    cometVideoApi.coolPhone.on('registered', function(e)
    {
        console.info("registered", e)
        if(callback) callback()
    });

    cometVideoApi.coolPhone.on('unregistered', function(e)
    {
        console.info("unregistered", e)
    });

    cometVideoApi.coolPhone.on('registrationFailed', function(e)
    {
        console.info("registrationFailed", e)
        if(cometVideoApi.opt.activeCallEvent)
        {
            cometVideoApi.callEnd(cometVideoApi.opt.activeCallEvent.data.sys.callPipe, {action:"terminated", status:"registration-failure"})
            cometVideoApi.hangup(cometVideoApi.opt.activeCallEvent)
        }
    });

    cometVideoApi.coolPhone.on('registrationFailure', function(e)
    {
        console.info("registrationFailure", e)
    });

    cometVideoApi.coolPhone.on('call', function(display_name, uri, call)
    {
        console.info("call", display_name, uri, call)
    });

    cometVideoApi.coolPhone.start();
}

/**
 * sends SIP REGISTER (expires=0) to logout
 * @private
 * @returns {Boolean}
 */
cometVideoApi._sipUnRegister = function(callback)
{ 
    //unregister()
    //stop()

    // Пока без UnRegister
    if(callback) callback()
    return false; 
}
  
/**
 * makes a call (SIP INVITE)
 * @param {type} type 'audio' | 'video'
 * @returns {undefined}
 *
 * @private
 */
cometVideoApi._sipCall = function(type, phoneNumber)
{
    if(!cometVideoApi.opt.activeCallEvent)
    {
        return;
    }

    cometVideoApi.opt.activeCallEvent.call_options.call.type = type
    cometVideoApi.opt.activeCallEvent.call_options.call.number = phoneNumber

    console.log('sip calling', type, phoneNumber);
 
    if( !cometVideoApi.coolPhone.isRegistered() || !cometVideoApi.coolPhone.isConnected())
    {
        setTimeout(cometVideoApi._sipCall, 500, type, phoneNumber)
        return;
    }

    var mediaConstraints = {'audio': true, 'video': type == 'video'}
 
    // запрашиваем разрешение на доступ к поточному видео камеры
    navigator.getUserMedia(mediaConstraints, function(stream)
    {
        cometVideoApi._sipCall_getUserMedia_success(stream, mediaConstraints, phoneNumber)
    }, function(stream){
        cometVideoApi._sipCall_getUserMedia_error(stream, mediaConstraints)
    });

}
    
cometVideoApi._sipCall_getUserMedia_success = function (stream, mediaConstraints, phoneNumber)
{
    console.log("Получили доступ к стриму", mediaConstraints)
    cometVideoApi.opt.activeCallEvent.callStream = stream
    var options = {
        'eventHandlers': eventHandlers,
        //'extraHeaders': [ ],
        'mediaConstraints': mediaConstraints,
        'rtcOfferConstraints' : { offerToReceiveAudio : true, offerToReceiveVideo : true },
        'mediaStream': stream, 
        'pcConfig': {
            'iceServers': [{'urls': ['stun:stun.l.google.com:19302']}]
        }
    };

    // Register callbacks to desired call events
    var eventHandlers = {
        // https://github.com/Ojero/jssip-demos/blob/master/tryit/js/gui.js
        'ring':   function(e){ console.info("ring", e) },
        'failure':   function(e){ console.info("failure", e) },
        'answer':   function(e){ console.info("answer", e) },
        'terminate':   function(e){ console.info("terminate", e) },
        'error':   function(e){ console.info("error", e) },
        'progress':   function(data){
            console.info("progress", data)
            // if (data.originator === 'remote') data.response.body = null; // https://groups.google.com/forum/#!topic/jssip/5lcrIREWYwU
        },
        'hold':   function(e){ console.info("hold", e) },
        'unhold':   function(e){ console.info("unhold", e) },
        'confirmed':  function(e)
        {
            // Attach local stream to selfView
            console.info("confirmed", e)

            //cometVideoApi.opt.activeCallEvent.call_options.video_local.src = window.URL.createObjectURL(session.connection.getLocalStreams()[0]);
            //selfView.src = window.URL.createObjectURL(session.connection.getLocalStreams()[0]);
        },
        'addstream':  function(event)
        {
            eventaddstream = event
            console.info("addstream", event)
            if(!cometVideoApi.opt.activeCallEvent)
            {
                console.warn('Звонок уже не активен - peerconnection "addstream" event', event);
                return;
            }

            var intervalId = setInterval(function()
            {
                if(!cometVideoApi.opt.activeCallEvent)
                {
                    return;
                }

                if(!cometVideoApi.isCallStart(cometVideoApi.opt.activeCallEvent.data.sys.callPipe))
                {
                    return;
                }
                clearInterval(intervalId)
                console.info('set streem to video src', event);
                cometVideoApi.opt.activeCallEvent.call_options.video_remote.src = window.URL.createObjectURL(event.stream);

            }, 300)

            // onactive
            // onaddtrack
            // onended
            // oninactive
            // onremovetrack

            cometVideoApi.callStart(cometVideoApi.opt.activeCallEvent.data.sys.callPipe, "self")
        },
        'ended':      function(e)
        {
            console.info("ended", e)
        }
    };

    session = cometVideoApi.coolPhone.call('sip:'+phoneNumber+'@'+cometVideoApi.opt.activeCallEvent.data.sys.serverName, options);
    for(var i in eventHandlers)
    {
        session.on(i, eventHandlers[i]);
    }

    session.on('peerconnection', function(data) {
        console.info("peerconnection", data)
    });
    session.on('progress', function(data) {
        console.info("progress", data)
    });
    session.on('accepted', function(data) {
        console.info("accepted", data)
    });
    session.on('confirmed', function(data) {
        console.info("confirmed", data)
    });
    session.on('failed', function(data)
    {
        console.error("failed", data)
        // console.log(data.message.data)
        //return;
        // Вызов завершения звонка по причине не удачи в подключении
        tabSignal.emit('_cometVideoApi_terminated');
        tabSignal.emit("media_removestream")

        if(cometVideoApi.opt.activeCallEvent)
        {
            cometVideoApi.callEnd(cometVideoApi.opt.activeCallEvent.data.sys.callPipe, {action:"terminated", status:"terminated"})
            cometVideoApi.hangup(cometVideoApi.opt.activeCallEvent)
        }
    });

    session.on('ended', function(data) 
    {
        console.info("ended", data)
        tabSignal.emit("media_removestream")
        if(cometVideoApi.opt.activeCallEvent)
        {
            cometVideoApi.callEnd(cometVideoApi.opt.activeCallEvent.data.sys.callPipe, {action:"terminated", status:"terminated"})
        }
    });

    session.on('newDTMF', function(data) {
        console.info("newDTMF", data)
    });
    session.on('reinvite', function(data) {
        console.info("reinvite", data)
    });
    session.on('update', function(data) {
        console.info("update", data)
    });
    session.on('refer', function(data) {
        console.info("refer", data)
    });
    session.on('replaces', function(data) {
        console.info("replaces", data)
    });
    session.on('sdp', function(event) {

        // webrtc: bandwidth usage in chrome - https://github.com/muaz-khan/RTCMultiConnection/issues/23
        // bandwidth (@todo Обратить внимание на то чтобы параметр b=AS был, а если его нет то добавлялся под записью a=mid:audio и a=mid:video)
        // Проверить работу с b=CT:256 и b=AS:256 и сравнить.
        // http://stackoverflow.com/questions/16712224/how-to-control-bandwidth-in-webrtc-video-call

        console.info("sdp", event)
        console.info("sdp old", event.sdp)
        //event.sdp = event.sdp.replace(/b=AS:[0-9]+\r\n/g, "") // CT | AS  https://tools.ietf.org/html/rfc4566#section-5.8
        window.oldSdp = event.sdp;

        if(cometVideoApi.opt.bandwidth_video > 0)
        {
            event.sdp = event.sdp.replace(/b=AS:[0-9]+\r\n/g, "") 
        }

        if(cometVideoApi.opt.bandwidth_video > 0)
        {
            event.sdp = event.sdp.replace(/m=video(.*)\r\n/g, "m=video$1\r\nb=AS:"+cometVideoApi.opt.bandwidth_video+"\r\n")
        }
        if(cometVideoApi.opt.bandwidth_audio > 0)
        { 
            event.sdp = event.sdp.replace(/m=audio(.*)\r\n/g, "m=audio$1\r\nb=AS:"+cometVideoApi.opt.bandwidth_audio+"\r\n")
        }

        console.info("sdp set bandwidth", {audio:cometVideoApi.opt.bandwidth_audio, video:cometVideoApi.opt.bandwidth_video})

        console.info("sdp new", event.sdp)

    });
    session.on('getusermediafailed', function(data) {
        console.error("getusermediafailed", data)
    });

    session.on('peerconnection:setremotedescriptionfailed', function(data) {
        console.error("peerconnection:setremotedescriptionfailed")
        console.log("peerconnection:setremotedescriptionfailed", data)
    });

    console.info("call session", session)
    var peerconnection = session.connection;

    //var localStream = peerconnection.getLocalStreams()[0];
    //var remoteStream = peerconnection.getRemoteStreams()[0];

    /*peerconnection.addEventListener('addstream', function(event)
    {
        console.info('peerconnection "addstream" event', event);
    });*/

}

cometVideoApi._sipCall_getUserMedia_error = function(mediaConstraints)
{
    console.log('что-то не так с медиастримом или пользователь запретил его использовать', mediaConstraints);
    // Вызов завершения звонка по причине не удачи в подключении
    tabSignal.emit('_cometVideoApi_terminated');

    if(cometVideoApi.opt.activeCallEvent)
    {
        cometVideoApi.callEnd(cometVideoApi.opt.activeCallEvent.data.sys.callPipe, {action:"terminated", status:"terminated"})
        cometVideoApi.hangup(cometVideoApi.opt.activeCallEvent)
    }
}

/**
 * transfers the call
 * @private
 **/
cometVideoApi._sipTransfer = function(s_destination)
{
    if (cometVideoApi._oSipSessionCall)
    {
        if (!tsk_string_is_null_or_empty(s_destination))
        {
            if (cometVideoApi._oSipSessionCall.transfer(s_destination) != 0)
            {
                console.error('Call transfer failed');
                return;
            }
            console.log('Transfering the call...');
        }
    }
}

/**
 * holds or resumes the call
 * @private
 * @returns {undefined|bolean} isHold
 **/
cometVideoApi.toggleHold = function()
{
    if(cometVideoApi.opt.activeCallEvent && cometVideoApi.opt.activeCallEvent.jssipRTCSession && cometVideoApi.opt.activeCallEvent.jssipRTCSession.isOnHold())
    {
        cometVideoApi.opt.activeCallEvent.jssipRTCSession.hold({audio: true, video:true})
    }
    else if(cometVideoApi.opt.activeCallEvent && cometVideoApi.opt.activeCallEvent.jssipRTCSession)
    {
        cometVideoApi.opt.activeCallEvent.jssipRTCSession.unhold({audio: true, video:true})
    }
    return undefined
}


/**
 * Определение состояния влага mute 
 * @public
 * @returns {undefined|{audio:bool, video:bool}} isMute
 **/
cometVideoApi.isMute = function()
{
    if(cometVideoApi.opt.activeCallEvent && cometVideoApi.opt.activeCallEvent.jssipRTCSession)
    {
        return cometVideoApi.opt.activeCallEvent.jssipRTCSession.isMuted() 
    }
    return undefined
}

/**
 * Ввключение/Выключение исходящего медиа потока
 * Mute or Unmute the call
 * @public
 * @returns {undefined|bolean} isMute
 **/
cometVideoApi.toggleMute = function()
{
    if(cometVideoApi.opt.activeCallEvent && cometVideoApi.opt.activeCallEvent.jssipRTCSession)
    {
        if(!cometVideoApi.opt.activeCallEvent.jssipRTCSession.isMuted().audio)
        {
            console.log("mute")
            cometVideoApi.opt.activeCallEvent.jssipRTCSession.mute({audio: true, video:true})
        }
        else
        {
            console.log("unmute")
            cometVideoApi.opt.activeCallEvent.jssipRTCSession.unmute({audio: true, video:cometVideoApi.opt.activeCallEvent.call_options.call.type == 'video'})
        }

        return cometVideoApi.opt.activeCallEvent.jssipRTCSession.isMuted().audio
    }
    return undefined
}

/**
 * Выключение/Включение звука или видео по отдельности.
 **/
cometVideoApi.muteAudio = function(setMute)
{
    if(cometVideoApi.opt.activeCallEvent && cometVideoApi.opt.activeCallEvent.jssipRTCSession)
    {
        console.log("mute audio")
        if(setMute)
        {
            cometVideoApi.opt.activeCallEvent.jssipRTCSession.mute({audio: true})
        }
        else
        {
            cometVideoApi.opt.activeCallEvent.jssipRTCSession.unmute({audio: true})
        }

        // Уведомляем о toggleMute
        cometApi.multi_web_pipe_send(cometVideoApi.opt.activeCallEvent.data.sys.callPipe, "toggleMute", { isMute:setMute })
        return true
    }
    return undefined
}

/**
 * Выключение/Включение звука или видео по отдельности.
 **/
cometVideoApi.muteVideo = function(setMute)
{
    if(cometVideoApi.opt.activeCallEvent && cometVideoApi.opt.activeCallEvent.jssipRTCSession)
    {
        console.log("mute audio")
        if(setMute)
        {
            cometVideoApi.opt.activeCallEvent.jssipRTCSession.mute({video: true})
        }
        else
        {
            cometVideoApi.opt.activeCallEvent.jssipRTCSession.unmute({video: true})
        }

        // Уведомляем о toggleMute
        cometApi.multi_web_pipe_send(cometVideoApi.opt.activeCallEvent.data.sys.callPipe, "toggleMute", { isMute:setMute })
        return true
    }
    return undefined
}


/**
 * Отклонение входящего вызова чтоб вызывающий абонент не ждал или положить трубку если мы в разговоре
 * @public
 * @param {object} callInfo
 * @param {boolean} doNotNotify не уведомлять собеседников о выходе
 **/
cometVideoApi.hangup = function(callInfo, doNotNotify)
{
    console.info("hangup", callInfo)

    var call = callInfo
    if(!callInfo)
    {
        call = cometVideoApi.opt.activeCallEvent
        cometVideoApi.opt.activeCallEvent = undefined
    }

    if(!call)
    {
        // Второй шанс проверяем нет ли ошибочно не закрытых соединений.
        // По правде где то была ошибка если этот код выполняется.
        for(var i in cometVideoApi._callPipes)
        {
            if(cometVideoApi._callPipes[i].isAccept && !cometVideoApi._callPipes[i]._isHungup)
            {
                call = cometVideoApi._callPipes[i]
                break;
            }
        }
    }

    if(call && call.selfStream && call.selfStream.getTracks)
    { 
        AudioTracks =  call.selfStream.getAudioTracks();

        AudioTracks.forEach(function(track) {
          track.stop();
        });
        
        VideoTracks =  call.selfStream.getVideoTracks();

        VideoTracks.forEach(function(track) {
          track.stop();
        });
    }
     
    if(call && call.callStream && call.callStream.getTracks)
    { 
        AudioTracks =  call.callStream.getAudioTracks();

        AudioTracks.forEach(function(track) {
          track.stop();
        });
        
        VideoTracks =  call.callStream.getVideoTracks();

        VideoTracks.forEach(function(track) {
          track.stop();
        });
    }

    // Мы выходим из звонка и уведомляем об этом
    if(call)
    {
        var callPipeName = call.data.sys.callPipe

        if(!doNotNotify)
        {
            // Уведомляем что скинули трубку
            cometApi.multi_web_pipe_send(callPipeName, "out", { status:"hangup", callKey:call.data.sys.callKey})
        }
        // Отпишимся от старого канала
        if(call._callPipeId) cometApi.unsubscription(call._callPipeId)
        if(call._trackPipeId) cometApi.unsubscription(call._trackPipeId)

        cometVideoApi.pingCancel(callPipeName)
    }
    else if(cometVideoApi.opt.lastCallEvent && !cometVideoApi.opt.lastCallEvent._isHangup)
    {
        // Положить трубку для звонка на который мы не ответили ещё и он не в статусе activeCallEvent
        cometVideoApi.opt.lastCallEvent._isHangup = true;
        var callPipeName = cometVideoApi.opt.lastCallEvent.data.sys.callPipe

        if(!doNotNotify)
        {
            cometApi.multi_web_pipe_send(callPipeName, "out", { status:"hangup", callKey:cometVideoApi.opt.lastCallEvent.data.sys.callKey})
        }
        if(cometVideoApi.opt.lastCallEvent.callPipe) cometApi.unsubscription(cometVideoApi.opt.lastCallEvent.callPipe)
        if(cometVideoApi.opt.lastCallEvent._trackPipeId) cometApi.unsubscription(cometVideoApi.opt.lastCallEvent._trackPipeId)

        cometVideoApi.pingCancel(cometVideoApi.opt.lastCallEvent.callPipe)
    }

    if(call && call.isAccept)
    { 
        cometVideoApi.callEnd(call.data.sys.callPipe, {action:"terminated", status:"hangup"})
        if(call.jssipRTCSession)
        {
            try{
                call.jssipRTCSession.terminate()
            }catch (e)
            {
                console.warn("exeption", e)
            }
        }

        cometVideoApi._sipUnRegister()
        cometVideoApi.opt.activeCallEvent = undefined
    }
}

/**
 * Отправляет DTMF в SIP
 * @example sipSendDTMF(&#39;1&#39;);
 * @private
 */
cometVideoApi._sendDTMF = function(c)
{
    if (cometVideoApi._oSipSessionCall && c)
    {
        if (cometVideoApi._oSipSessionCall.dtmf(c) == 0)
        {
            return true;
        }
    }
    return false;
}

/**
 * Вернёт строку для значения заголовка language
 * @returns {String}
 */
cometVideoApi._languageHeader = function(){
    return '"' + cometVideoApi.opt.language.join(",") + '"'
}

/**
 * Ставит настройки языка пользователя
 * @param {array} language
 **/
cometVideoApi._setLanguageHeader = function(language)
{
    cometVideoApi.opt.language = language
}

/**
 * Переключение в полноэкранный режим
 */
cometVideoApi.fullScreen = function(elem)
{
    if(elem == undefined)
    {
        if(!cometVideoApi.opt.activeCallEvent)
        {
            return;
        }
        elem = cometVideoApi.opt.activeCallEvent.call_options.video_remote
    }

    if (elem.requestFullscreen)
    {
        elem.requestFullscreen();
    }
    else if (elem.mozRequestFullScreen)
    {
        elem.mozRequestFullScreen();
    }
    else if (elem.webkitRequestFullscreen)
    {
        elem.webkitRequestFullscreen();
    }
}

/**
 * Определение поддерживается ли полноэкранный режим
 */
cometVideoApi.isFullScreenSupport = function()
{
    var elem = document.createElement("video");

    if (elem.requestFullscreen)
    {
        return true;
    }
    else if (elem.mozRequestFullScreen)
    {
        return true;
    }
    else if (elem.webkitRequestFullscreen)
    {
        return true;
    }

    return false;
}
