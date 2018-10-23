

var mysql = require('mysql');

var host1 = "127.0.0.1";
var port1 = "3307";
var portws1 = "8087";


/* I used for creating tokens https://jwt.io/
 * Secret string is `CometQLPassword1` it is `password_string + dev_id` from CometQL connection
 *
HEADER:
{
  "alg": "HS256",
  "typ": "JWT"
}
PAYLOAD:
{
  "user_id": 5,
  "exp": 9983484000
}
 *
 * @type String
 */
var validJWT   = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjo1LCJleHAiOjk5ODMzMTEyMDB9.jXeCiNOA1eKLiLwcA2bkyaFZOhewvt81FCIfF278aO8";
var revokedJWT = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjo1LCJleHAiOjk5ODM0ODQwMDB9.8VU1bBHa0sFVOqH19tnV3YOy7Y23aOCaCpMDzqAB64g";
var invalidJWT   = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoyLCJleHAiOjk5ODMzMTEyMDB9.aIh8RCsv1W5dHmTNVDKfHxVaerpuLBz0l-5IBLIhro0";
var maxTimeTimeout = 28000

// mysql -h127.0.0.1 -uroot -pCometQLPassword -DCometQL_v1 -P3311 --skip-ssl
var connection1 = mysql.createConnection({
  host     : host1,
  port     : port1,
  user     : '1',
  password : 'CometQLPassword',
  database : 'CometQL_v1'
});
console.log("\x1b[1;32m[mysql string]\x1b[1;33mmysql -h"+host1+" -u1 -pCometQLPassword -DCometQL_v1 -P"+port1+" --skip-ssl\x1b[0m");

connection1.on('error', function(err) {
    console.log("\x1b[1;31m[mysql error]\x1b[0m", err);
    //throw new Error(err);
});

connection1.on('_onPacket', function(err) {
    console.log("\x1b[1;31m[mysql error]\x1b[0m", err);
    //throw new Error(err);
});
connection1.connect();

var apiTest1 = new cometServerApi();
apiTest1.setLogLevel(9)
apiTest1.start({dev_id:1, user_id:1, user_key:"not00000valid00000userHash", node:[host1 + ":" + portws1]})

var apiTest2 = new cometServerApi();
apiTest2.setLogLevel(9)
apiTest2.start({dev_id:1, user_id:2, user_key:"not00000valid00000userHash", node:[host1 + ":" + portws1]})

var apiTest3 = new cometServerApi();
apiTest3.setLogLevel(9)
apiTest3.start({dev_id:1, user_id:13, user_key:"not00000valid00000userHash", node:[host1 + ":" + portws1]})

var apiTest4 = new cometServerApi();
apiTest4.setLogLevel(9)
apiTest4.start({dev_id:1, user_id:14, user_key:"not00000valid00000userHash", node:[host1 + ":" + portws1]})

/*var apiTest3 = new cometServerApi();
apiTest3.setLogLevel(9)
apiTest3.start({dev_id:0, user_id:1, user_key:"not00000valid00000userHash", node:["localhost:8087"]})

var apiTest4 = new cometServerApi();
apiTest4.setLogLevel(9)
apiTest4.start({dev_id:0, user_id:2, user_key:"not00000valid00000userHash", node:["localhost:8088"]})
*/

if(apiTest1.getLogLevel() != 9)
{
    throw new Error("[js-test] Error in getLogLevel/setLogLevel");
}

if(apiTest1.getTabUUID() != apiTest1.getTabUUID())
{
    throw new Error("[js-test] Error: getTabUUID return differents values");
}



if(apiTest1.getUserId() != 1)
{
    throw new Error("[js-test] Error in getUserId is `"+apiTest1.getUserId() +"`");
}

if(apiTest1.getUserKey() != 'not00000valid00000userHash')
{
    throw new Error("[js-test] Error in getUserKey is not `not00000valid00000userHash`");
}

if(apiTest1.getDevId() != 1)
{
    throw new Error("[js-test] Error in getDevId is not `0`");
}

if(apiTest1.UseWss() != '')
{
    throw new Error("[js-test] Error in UseWss is not empty");
}

if(apiTest1.isUseWss())
{
    throw new Error("[js-test] Error in isUseWss is not false");
}

function test_realUserKey()
{
    if(!apiTest1.setRealUserKey("abc"))
    {
        throw new Error("[js-test] Error in setRealUserKey is not true");
    }

    if(apiTest1.getRealUserKey() != "abc")
    {
        throw new Error("[js-test] Error in getRealUserKey is not `abc`");
    }
    console.log("[js-test] \x1b[1;32m getRealUserKey/setRealUserKey ok\x1b[0m");
}

var apiWithAuth = new cometServerApi();
apiWithAuth.setLogLevel(9)

function test_2_users_messages()
{
    var juserdata = {
        rand:Math.random()+""
    }

    var query = "DELETE FROM users_messages WHERE id = 99955;";
    connection1.query(query,
        function(error, result, fields)
        {
            query = "INSERT INTO users_messages (id, event, message)VALUES (99955, 'test1_users_messages', '"+JSON.stringify(juserdata)+"');";
            connection1.query(query,
                function(error, result, fields)
                {
                    if(error)
                    {
                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                    }

                    query = "select * from users_messages where id = 99955";
                    connection1.query(query,
                        function(error, result, fields)
                        {
                            if(error)
                            {
                                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                            }

                            if(result.length != 1)
                            {
                                throw new Error(JSON.stringify({test:"[js-test] Error (result.length != 3) in query:"+query, error:error, result:result, fields:fields }));
                            }

                            query = "DELETE FROM users_messages WHERE id = 99955;";
                            connection1.query(query,
                                function(error, result, fields)
                                {
                                    if(error)
                                    {
                                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                                    }

                                    query = "select * from users_messages where id = 99955";
                                    connection1.query(query,
                                        function(error, result, fields)
                                        {
                                            if(error)
                                            {
                                                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                                            }

                                            if(result.length != 0)
                                            {
                                                throw new Error(JSON.stringify({test:"[js-test] Error[2] (result.length != 0) in query:"+query, error:error, result:result, fields:fields }));
                                            }
                                        }
                                    );
                                }
                            );
                        }
                    );

                }
            );
        }
    );
}

function test_short_users_messages()
{
    var isTestDone = 0;
 
    let msg = ""
    for(let i=0; i<60; i++)
    {
        msg+=""+i%9;
        let query = "INSERT INTO users_messages (id, event, message)VALUES (3, 'user', '"+msg+"');";
        connection1.query(query,
            function(error, result, fields)
            {
                if(error)
                {
                    throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                } 
            }
        ); 
    }
    
    apiWithAuth.subscription("msg.user", function(event)
    {
        isTestDone += 1
        console.log("[js-test] \x1b[1;32m test_short_users_messages (apiWithAuth.subscription test_short_users_messages) ok\x1b[0m"); 
    })

    setTimeout(function()
    {
        if(isTestDone != 60)
        {
            throw new Error("[js-test] Error test_short_users_messages not done (isTestDone="+isTestDone+")");
        }
    }, 58000)
}

/**
 *
 * use db_35
 * select * from users_time where id in(1, 2, 99996);
 * use db_1
 * select * from users_time where id in(1, 2, 99996);
 *
 *
 * +-------+------------+
 * | id    | time       |
 * +-------+------------+
 * | 1     | -1         |
 * | 2     | 1528249687 |
 * | 99996 | -1         |
 * +-------+------------+
 * 3 rows in set (0,86 sec)
 *
 */

function test_users_messages()
{
    apiWithAuth.onAuthSuccess(function()
    {
        setTimeout(function(){
            var query = "select * from users_time where id in(1, 3, 99996);";
            connection1.query(query,
                function(error, result, fields)
                {
                    if(error)
                    {
                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                    }

                    if(result.length != 3)
                    {
                        throw new Error(JSON.stringify({test:"[js-test] Error (result.length != 3) in query:"+query, error:error, result:result, fields:fields }));
                    }

                    var usersTime = {}
                    for(var i =0; i< result.length; i++)
                    {
                        usersTime[result[i].id] = result[i];
                    }


                    if(usersTime[3].time != 0)
                    {
                        throw new Error(JSON.stringify({test:"[js-test] Error (usersTime[3].time != 0) in query:"+query, usersTime:usersTime }));
                    }

                    if(usersTime[99996].time != -1)
                    {
                        throw new Error(JSON.stringify({test:"[js-test] Error (usersTime[99996].time != -1) in query:"+query, usersTime:usersTime }));
                    }
                    console.log("[js-test] \x1b[1;32m users_time ok\x1b[0m");
                }
            );
        }, 5000) 
    })
}

function test_onAuth2()
{
    var isTestDone = 0;

    var juserdata = {
        rand:Math.random()+""
    }

    apiWithAuth.subscription("msg.test_onAuth2", function(event)
    {
        if(event.data.rand != juserdata.rand)
        {
            throw new Error("[js-test] Error test_users_messages not done ( event.data.rand != juserdata.rand )", event);
        }
        isTestDone += 1
        console.log("[js-test] \x1b[1;32m test_onAuth2 (apiWithAuth.subscription test_onAuth2) ok\x1b[0m"); 
    })

    var query = "DELETE FROM users_messages WHERE id = 3;";
    connection1.query(query,
        function(error, result, fields)
        {
            query = "INSERT INTO users_messages (id, event, message)VALUES (3, 'test_onAuth2', '"+JSON.stringify(juserdata)+"');";
            connection1.query(query,
                function(error, result, fields)
                {
                    if(error)
                    {
                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                    }
                    // добавить проверку селекта из users_messages
                    // перенести сюда код от строки 295
                    query = 'select * from users_messages where id = 3';
                    connection1.query(query,
                        function(error, result, fields)
                        {
                            if(error)
                            {
                                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                            }

                            if(result.length != 1)
                            {
                                throw new Error(JSON.stringify({test:"[js-test] Error ( result.length != 1 ) in query:"+query, error:error, result:result, fields:fields }));
                            }

                            query = 'delete from users_auth where id = 3';
                            connection1.query(query,
                                function(error, result, fields)
                                {
                                    if(error)
                                    {
                                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                                    }

                                    query = "INSERT INTO users_auth (id, hash)VALUES (3, 'valid00000userHash');";
                                    connection1.query(query,
                                        function(error, result, fields)
                                        {
                                            if(error)
                                            {
                                                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                                            }
                                            query = 'select * from users_auth where id = 3';
                                            connection1.query(query,
                                                function(error, result, fields)
                                                {
                                                    if(error)
                                                    {
                                                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                                                    }

                                                    if(result.length != 1)
                                                    {
                                                        throw new Error(JSON.stringify({test:"[js-test] Error ( result.length != 1 ) in query:"+query, error:error, result:result, fields:fields }));
                                                    }
                                                    console.log("[js-test] \x1b[1;32m test_onAuth2 hash=\x1b[0m", result[0]);
                                                    apiWithAuth.start({dev_id:1, user_id:3, user_key:result[0].hash, node:[host1 + ":" + portws1]})
                                                }
                                            );
                                        }
                                    );
                                }
                            );
                        }
                    );
                }
            );
        }
    );

    apiWithAuth.onAuthSuccess(function()
    {
        isTestDone += 1;
        console.log("[js-test] \x1b[1;32m test_onAuth2 (onAuthSuccess) ok\x1b[0m");
    })

    apiWithAuth.onAuthFalill(function()
    {
        throw new Error("[js-test] Error onAuthFalill in test_onAuth2");
    })

    setTimeout(function()
    {
        if(isTestDone != 2)
        {
            throw new Error("[js-test] Error test_onAuth2 not done (isTestDone="+isTestDone+")");
        }
    }, 28000)
}

var apiWithJWT_Auth = new cometServerApi();
apiWithJWT_Auth.setLogLevel(9)

function test_JWT_Auth()
{
    var isTestDone = false;

    var query = 'delete from revoked_tokens where token = "'+validJWT+'"';
    connection1.query(query,
        function(error, result, fields)
        {
            if(error)
            {
                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
            }

            apiWithJWT_Auth.start({dev_id:1, user_id:5, user_key:validJWT, node:[host1 + ":" + portws1]})
        }
    );

    apiWithJWT_Auth.onAuthSuccess(function()
    {
        isTestDone = true;
        console.log("[js-test] \x1b[1;32m test_JWT_Auth ok\x1b[0m");
    })

    apiWithJWT_Auth.onAuthFalill(function()
    {
        throw new Error("[js-test] Error (test_JWT_Auth) onAuthFalill()");
    })

    setTimeout(function()
    {
        if(!isTestDone)
        {
            throw new Error("[js-test] Error test_JWT_Auth not done");
        }
    }, maxTimeTimeout)
}

var apiWithRevokedJWT_Auth = new cometServerApi();
apiWithRevokedJWT_Auth.setLogLevel(9)

function test_Revoked_JWT_table()
{
    var isTestDone = false;
    var revokedJWT = Math.random()+"";
    
    
    var query = 'insert into revoked_tokens(token)VALUES("'+revokedJWT+'");';

    console.log("[js-test] \x1b[1;33m test_Revoked_JWT_tables:\x1b[0m", query);
    connection1.query(query,
        function(error, result, fields)
        {
            if(error)
            {
                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
            }
 
            query = 'select * from  revoked_tokens where token = "'+revokedJWT+'"';
            connection1.query(query,
                function(error, result, fields)
                {
                    if(error)
                    {
                        throw new Error("[js-test] Error in query:"+query);
                    }

                    if(result.length != 1)
                    {
                        console.log("Error in query:"+query, error, result, fields);
                        throw new Error("[js-test] Error in query (test_Revoked_JWT_table 1):"+query+", result.length = " + result.length);
                    }

                    if(result[0].token != revokedJWT)
                    {
                        console.log("Error in query:"+query, error, result, fields);
                        throw new Error("[js-test] Error in query:"+query+", result[0].id != user_id");
                    }
 
                    
                    query = 'delete from  revoked_tokens where token = "'+revokedJWT+'"';
                    connection1.query(query,
                        function(error, result, fields)
                        {
                            if(error)
                            {
                                throw new Error("[js-test] Error in query:"+query);
                            }
 
                            query = 'select * from revoked_tokens where token = "'+revokedJWT+'"';
                            connection1.query(query,
                                function(error, result, fields)
                                {
                                    if(error)
                                    {
                                        throw new Error("[js-test] Error in query:"+query);
                                    }

                                    if(result.length != 0)
                                    {
                                        console.log("Error in query:"+query, error, result, fields);
                                        throw new Error("[js-test] Error in query  (test_Revoked_JWT_table 2) :"+query+", result.length = " + result.length);
                                    } 
                                    
                                    isTestDone = true
                            })
                    })
            })
    })
            
   
 
    setTimeout(function()
    {
        if(!isTestDone)
        {
            throw new Error("[js-test] Error test_Revoked_JWT_table not done");
        }
    }, maxTimeTimeout)
}


function test_Revoked_JWT_Auth()
{
    var isTestDone = false;

    var query = 'insert into revoked_tokens(token)VALUES("'+revokedJWT+'");';

        console.log("[js-test] \x1b[1;33m test_Revoked_JWT_Auth:\x1b[0m", query);
    connection1.query(query,
        function(error, result, fields)
        {
            if(error)
            {
                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
            }

            apiWithRevokedJWT_Auth.start({dev_id:1, user_id:6, user_key:revokedJWT, node:[host1 + ":" + portws1]})
        }
    );

    apiWithRevokedJWT_Auth.onAuthSuccess(function()
    {
        throw new Error("[js-test] Error test_Revoked_JWT_Auth onAuthSuccess()");
    })

    apiWithRevokedJWT_Auth.onAuthFalill(function()
    {
        isTestDone = true;
        console.log("[js-test] \x1b[1;32m test_Revoked_JWT_Auth ok\x1b[0m");
    })

    setTimeout(function()
    {
        if(!isTestDone)
        {
            throw new Error("[js-test] Error test_Revoked_JWT_Auth not done");
        }
    }, maxTimeTimeout)
}

var apiWithinvalidJWT_Auth = new cometServerApi();
apiWithinvalidJWT_Auth.setLogLevel(9)

function test_invalid_JWT_Auth()
{
    var isTestDone = false;

    apiWithinvalidJWT_Auth.start({dev_id:1, user_id:12, user_key:invalidJWT, node:[host1 + ":" + portws1]})
    apiWithinvalidJWT_Auth.onAuthSuccess(function()
    {
        throw new Error("[js-test] Error test_Revoked_JWT_Auth onAuthSuccess()");
    })

    apiWithinvalidJWT_Auth.onAuthFalill(function()
    {
        isTestDone = true;
        console.log("[js-test] \x1b[1;32m test_Revoked_JWT_Auth ok\x1b[0m");
    })

    setTimeout(function()
    {
        if(!isTestDone)
        {
            throw new Error("[js-test] Error test_Revoked_JWT_Auth not done");
        }
    }, maxTimeTimeout)
}

function test_onAuth()
{
    var isTestDone = false;
    apiTest1.onAuthSuccess(function()
    {
        console.log("[js-test] onAuthSuccess");

        if(apiTest1.isAuthorized())
        {
            console.log("[js-test] isAuthorized true");
        }

        throw new Error("[js-test] Error (test_onAuth) onAuthSuccess()");
    })

    apiTest1.onAuthFalill(function()
    {
        console.log("[js-test] onAuthFalill");
        if(!apiTest1.isAuthorized())
        {
            console.log("[js-test] isAuthorized false");
        }

        if(!apiTest1.UseWebSocket())
        {
            throw new Error("[js-test] Error UseWebSocket()");
        }

        if(!apiTest1.isMaster())
        {
            throw new Error("[js-test] Error isMaster()");
        }

        console.log("[js-test] \x1b[1;32m test_onAuth ok\x1b[0m");
        isTestDone = true;
    })


    setTimeout(function()
    {
        if(!isTestDone)
        {
            throw new Error("[js-test] Error test_onAuth not done");
        }
    }, maxTimeTimeout)
}

function test_getTrackPipeUsers()
{
    var isTestDone = false;

    if(apiTest1.getTrackPipeUsers("test1", function(){}))
    {
        throw new Error("[js-test] Error getTrackPipeUsers allow track for pipe `test1`");
    }

    apiTest1.getTrackPipeUsers("track_test1", function(event)
    {
        if(!Array.isArray(event.data))
        {
            throw new Error("[js-test] Error getTrackPipeUsers filed `data` not array");
        }

        if(!event.data)
        {
            throw new Error("[js-test] Error getTrackPipeUsers filed `data` not empty");
        }

        if(!event.server_info)
        {
            throw new Error("[js-test] Error getTrackPipeUsers filed `server_info` is empty");
        }

        console.log("[js-test] \x1b[1;32m test_getTrackPipeUsers ok\x1b[0m");
        isTestDone = true;

    })

    setTimeout(function()
    {
        if(!isTestDone)
        {
            throw new Error("[js-test] Error getTrackPipeUsers not done");
        }
    }, maxTimeTimeout)
}

function test_getUserData()
{
    var isTestDone = false;
    apiTest1.getUserData(99999-1, function(event)
    {
        if(Array.isArray(event.data) || typeof event.data != "object" )
        {
            throw new Error("[js-test] Error getUserData filed `data` is not object");
        }

        if(event.data.user_data)
        {
            throw new Error("[js-test] Error getUserData filed `user_data` not empty");
        }

        if(!event.data.user_id)
        {
            throw new Error("[js-test] Error getUserData filed `user_id` not "+99999-1);
        }

        if(!event.server_info)
        {
            throw new Error("[js-test] Error getUserData filed `server_info` is empty");
        }

        isTestDone = true;
        console.log("[js-test] \x1b[1;32m test_getUserData ok\x1b[0m");

    })

    setTimeout(function()
    {
        if(!isTestDone)
        {
            throw new Error("[js-test] Error test_getUserData not done isTestDone="+isTestDone);
        }
    }, maxTimeTimeout)
}

function test_setUserData(user_id, rdata)
{
    var isTestDone = 0;


    var juserdata = {
        rand:rdata+""
    }

    var query = 'insert into users_data(id, data)VALUES("'+user_id+'", \''+JSON.stringify(juserdata)+'\');';
    connection1.query(query,
        function(error, result, fields)
        {
            if(error)
            {
                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
            }
            isTestDone += 1;
            query = 'select * from  users_data where id = '+user_id;
            console.log("Test query:"+query );
            
            connection1.query(query,
                function(error, result, fields)
                {
                    isTestDone += 1;
                    if(error)
                    {
                        throw new Error("[js-test] Error in query:"+query);
                    }

                    if(result.length != 1)
                    {
                        console.log("Error in query:"+query, error, result, fields);
                        throw new Error("[js-test] Error in query:"+query+", result.length = " + result.length);
                    }

                    if(result[0].id != user_id)
                    {
                        console.log("Error in query:"+query, error, result, fields);
                        throw new Error("[js-test] Error in query:"+query+", result[0].id != user_id");
                    }

                    if(!result[0].data)
                    {
                        console.log("Error in query:"+query, error, result, fields);
                        throw new Error("[js-test] Error in query:"+query+", !result[0].data");
                    }

                    var data = JSON.parse(result[0].data)
                    if(data.rand != juserdata.rand)
                    {
                        console.log("Error in query:"+query, data.rand , juserdata.rand);
                        throw new Error("[js-test] Error in query:"+query+", data.rand != juserdata.rand");
                    }

                    //console.log("connection1 query", error, result, fields);

                    apiTest1.getUserData(user_id, function(event)
                    {
                        if(Array.isArray(event.data) || typeof event.data != "object" )
                        {
                            throw new Error("[js-test] Error getUserData filed `data` is not object");
                        }

                        if(!event.data.user_data)
                        {
                            throw new Error("[js-test] Error getUserData filed `user_data` is empty");
                        }

                        if(event.data.user_id != user_id)
                        {
                            throw new Error("[js-test] Error getUserData filed `user_id` not "+user_id);
                        }

                        if(event.data.user_data != '{"rand":"'+juserdata.rand+'"}')
                        {
                            console.log(event.data.user_data, '{"rand":"'+juserdata.rand+'"}');
                            throw new Error("[js-test] Error getUserData filed `user_data` != juserdata.rand ");
                        }

                        isTestDone += 1;
                        console.log("[js-test] \x1b[1;32m test_getUserData ok\x1b[0m");

                    })
                }
            );

        }
    );

    setTimeout(function()
    {
        if(isTestDone != 3)
        {
            throw new Error("[js-test] Error test_setUserData not done isTestDone="+isTestDone);
        }
    }, maxTimeTimeout)
}

function test_web_pipe_send()
{
    var send = 20;
    var isTestDone = 0;
    var rand = Math.floor(Math.random()*999999999);

    apiTest1.subscription("web_test.t1_"+rand, function(event){
         throw new Error("[js-test] Error `web_pipe_send` receive self send event");
    })

    apiTest2.subscription("web_test.t1"+rand, function(event){
        isTestDone += 1;
        console.log("[js-test] \x1b[1;32m test_web_pipe_send ok\x1b[0m");
    })

    apiTest2.subscription("web_test."+rand, function(event){
        isTestDone += 1;
        console.log("[js-test] \x1b[1;32m test_web_pipe_send ok\x1b[0m");
    })

    setTimeout(function()
    {
        // apiTest1.web_pipe_send("web_test."+rand, "test1", rand) // Ломает тест. Выяснить от чего.
        apiTest1.web_pipe_send("web_test.t1"+rand, rand)

        for(var i = 0; i< send; i++)
        {
            apiTest1.web_pipe_send("web_test."+rand, rand)
        }

        if(apiTest1.web_pipe_send("test1."+rand, rand))
        {
            throw new Error("[js-test] Error web_pipe_send allow send to pipe `test1`");
        }
    }, 1000)

    setTimeout(function()
    {
        if(isTestDone == send + 1)
        {
            throw new Error("[js-test] Error test_web_pipe_send not done");
        }
    }, maxTimeTimeout)
}

if(apiTest2.getTabUUID() != apiTest2.getTabUUID())
{ 
    throw new Error(JSON.stringify({error:"[js-test] Error apiTest2.getTabUUID() != apiTest2.getTabUUID()"})); 
}

if(apiTest2.getUUID() != apiTest2.getUUID())
{ 
    throw new Error(JSON.stringify({error:"[js-test] Error apiTest2.getUUID() != apiTest2.getUUID()"})); 
}

function test_track_subscription()
{
    var isTestDone = 0;

    var pipe_name = "track_test_"+Math.floor(Math.random()*100000);
    var laseEvent = undefined
    
    apiTest4.subscription(pipe_name, function(event){
        console.log("[js-test] \x1b[1;32m track_test_track_subscription isTestDone="+isTestDone+" pipe_name="+pipe_name+"\x1b[0m", event);
        
        if(event.server_info.event == "unsubscription")
        {
            return;
        }
         
        if(event.data.uuid != apiTest3.getUUID())
        {
            throw new Error(JSON.stringify({error:"[js-test] Error (event.data.uuid != apiTest3.getUUID()) test_track_subscription not done isTestDone="+isTestDone, 
                event:event,
                laseEvent:laseEvent,
                apiTest3_TabUUID:apiTest3.getUUID()
            })); 
        }
        
        isTestDone += 1;
        if(isTestDone > 1)
        {
            throw new Error(JSON.stringify({error:"[js-test] Error (isTestDone > 1) test_track_subscription not done isTestDone="+isTestDone, 
                event:event,
                laseEvent:laseEvent,
                apiTest3:apiTest3.getUUID()
            })); 
        }  
        
        laseEvent = event
    })

    setTimeout(function()
    {
        apiTest3.subscription(pipe_name, function(event){ 
            
            if(event.server_info.event == "unsubscription")
            {
                return;
            }

            throw new Error(JSON.stringify({error:"[js-test] Error (in apiTest3 ) test_track_subscription not done isTestDone="+isTestDone, event:event})); 
        })
    }, 2000)/**/

    setTimeout(function()
    {
        if(isTestDone != 1)
        {
            throw new Error("[js-test] Error (isTestDone == 1) test_track_subscription not done isTestDone="+isTestDone);
        }
    }, maxTimeTimeout)
}

function test_get_pipe_log()
{
    var rdata = Math.random()+"ABC"

    var juserdata = {
        rand:rdata+""
    }

    var pipe_name = "test_pipelog";
    var event_name = "e1";
    var isTestDone = 0;

    var query = 'INSERT INTO pipes_settings ("name", "length") VALUES ("'+pipe_name+'", 2);';
    connection1.query(query,
        function(error, result, fields)
        {
            if(error)
            {
                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
            }
            console.log("[js-test] \x1b[1;32m test_get_pipe_log 1\x1b[0m", result);

            query = 'insert into pipes_messages(name, event, message)VALUES("'+pipe_name+'", "'+event_name+'", \''+JSON.stringify(juserdata)+'\');';
            connection1.query(query,
                function(error, result, fields)
                {
                    if(error)
                    {
                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                    }

                    query = 'insert into pipes_messages(name, event, message)VALUES("'+pipe_name+'", "'+event_name+'", \''+JSON.stringify(juserdata)+'\');';
                    connection1.query(query,
                        function(error, result, fields)
                        {
                            if(error)
                            {
                                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                            }

                            apiTest1.subscription(pipe_name+"."+event_name, function(event){
                                isTestDone += 1;
                                console.log("[js-test] \x1b[1;32m test_get_pipe_log ok\x1b[0m");
                                // @todo добавить валидацию ответа
                            })


                            apiTest2.subscription(pipe_name+"."+event_name, function(event){
                                throw new Error(JSON.stringify({error:"[js-test] Error test_get_pipe_log apiTest2 get pipe log isTestDone="+isTestDone, event:event}));
                            })

                            apiTest1.get_pipe_log(pipe_name);
                        }
                    );
                }
            );
        }
    );


    setTimeout(function()
    {
        if(isTestDone < 2)
        {
            throw new Error("[js-test] Error test_get_pipe_log not done isTestDone="+isTestDone);
        }
    }, maxTimeTimeout)
}


function test_ql_subscription()
{
    var isTestDone = 0;

    var pipe_name = "tttest_"+Math.floor(Math.random()*10000);
    var messages = 20;  
    var event_name = "evt";
    var rdata = Math.random()+"ABC"

    var juserdata = {
        rand:rdata+""
    }

    apiTest1.subscription(pipe_name+"."+event_name, function(event)
    {
        if(event.data.rand != juserdata.rand)
        {
            throw new Error(JSON.stringify({error:"[js-test] Error test_ql_subscription not done ( event.data.rand != juserdata.rand )", event:event}));
        }
        
        isTestDone += 1;
        console.log("[js-test] \x1b[1;32m test_ql_subscription ok\x1b[0m");
    })
    
    var query;
    setTimeout(function()
    {
        query = 'SELECT * FROM users_in_pipes WHERE name = "'+pipe_name+'"; ';
        connection1.query(query,
            function(error, result, fields)
            {
                if(error)
                {
                    throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                }
                
                if(result.length != 1)
                {
                    throw new Error(JSON.stringify({test:"[js-test] Error (result.length != 1) in query:"+query, error:error, result:result, fields:fields }));
                }
                 
                if(result[0]["name"] != pipe_name)
                {
                    throw new Error(JSON.stringify({test:"[js-test] Error (result[0][name] != pipe_name) in query:"+query, error:error, result:result, fields:fields }));
                }
                
                if(result[0]["user_uuid"] != apiTest1.getUUID())
                {
                    throw new Error(JSON.stringify({test:"[js-test] Error (result[0][user_uuid] != apiTest1.getUUID()) in query:"+query, error:error, result:result, fields:fields }));
                }
                
                console.log("[js-test] \x1b[1;32m users_in_pipes\x1b[0m"); 
            }
        );

        query = 'SELECT * FROM pipes WHERE name = "'+pipe_name+'"; ';
        connection1.query(query,
            function(error, result, fields)
            {
                if(error)
                {
                    throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                }
                
                if(result.length != 1)
                {
                    throw new Error(JSON.stringify({test:"[js-test] Error (result.length != 1) in query:"+query, error:error, result:result, fields:fields }));
                }
                 
                if(result[0]["name"] != pipe_name)
                {
                    throw new Error(JSON.stringify({test:"[js-test] Error (result[0][name] != pipe_name) in query:"+query, error:error, result:result, fields:fields }));
                }
                
                if(result[0]["users"] != 1)
                {
                    throw new Error(JSON.stringify({test:"[js-test] Error (result[0][users] != 1) in query:"+query, error:error, result:result, fields:fields }));
                }
                
                console.log("[js-test] \x1b[1;32m users_in_pipes\x1b[0m"); 
            }
        );

        for(var i = 0; i< messages; i++)
        {
            juserdata.index = i

            // @todo добавить невалидные запросы и запросы в разных формах
            query = 'insert into pipes_messages(name, event, message)VALUES("'+pipe_name+'", "'+event_name+'", \''+JSON.stringify(juserdata)+'\');';
            connection1.query(query,
                function(error, result, fields)
                {
                    if(error)
                    {
                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                    }
                }
            );
        }
    }, 6000)

    setTimeout(function()
    {
        if(isTestDone != messages )
        {
            throw new Error("[js-test] Error test_ql_subscription not done isTestDone=" + isTestDone);
        }
    }, maxTimeTimeout)
}

function test_ql_pipes_settings()
{
    var isTestDone = 0;
    var pipe_name = "secr_"+Math.floor(Math.random()*1000);

    var query = 'INSERT INTO pipes_settings ("name", "length") VALUES ("'+pipe_name+'", 10);';
    connection1.query(query,
        function(error, result, fields)
        {
            if(error)
            {
                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
            }
            console.log("[js-test] \x1b[1;32m test_ql_pipes_settings 1\x1b[0m", result);

            query = 'SELECT * FROM pipes_settings WHERE name = "'+pipe_name+'" ';
            connection1.query(query,
                function(error, result, fields)
                {
                    if(error)
                    {
                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                    }

                    console.log("[js-test] \x1b[1;32m test_ql_pipes_settings 2\x1b[0m", result);

                    //  Normal answer: { name: 'secret_pipe', length: '10' }
                    if(result.length != 1)
                    {
                        throw new Error(JSON.stringify({test:"[js-test test_ql_pipes_settings] Error (more then one line in result) in query:"+query, error:error, result:result, fields:fields }));
                    }

                    if(result[0].length != "10")
                    {
                        throw new Error(JSON.stringify({test:"[js-test] Error (result[0].message != 10) in query:"+query, error:error, result:result, fields:fields }));
                    }

                    query = 'DELETE FROM pipes_messages WHERE name = "'+pipe_name+'" ';
                    connection1.query(query,
                        function(error, result, fields)
                        {
                            if(error)
                            {
                                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                            }

                            var text_message = "text message "+Math.random()
                            query = 'INSERT INTO pipes_messages (name, event, message)VALUES("'+pipe_name+'", "event_in_pipe", "'+text_message+'");';
                            connection1.query(query,
                                function(error, result, fields)
                                {
                                    if(error)
                                    {
                                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                                    }

                                    console.log("[js-test] \x1b[1;32m test_ql_pipes_settings 3\x1b[0m", result);
                                    query = 'SELECT * FROM pipes_messages WHERE name = "'+pipe_name+'" ';
                                    connection1.query(query,
                                        function(error, result, fields)
                                        {
                                            if(error)
                                            {
                                                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                                            }

                                            console.log("[js-test] \x1b[1;32m test_ql_pipes_settings 4\x1b[0m", result);
                                            /*  Normal answer:  {
                                                id: '5f358dd6-6733-11e8-a',
                                                time: '1528033097',
                                                name: 'secret_pipe',
                                                index: '0',
                                                event: 'event_in_pipe',
                                                message: 'text message',
                                                user_id: '0'
                                            }*/

                                            if(result.length != 1)
                                            {
                                                throw new Error(JSON.stringify({test:"[js-test test_ql_pipes_settings 2] Error (more then one line in result) in query:"+query, error:error, result:result, fields:fields }));
                                            }

                                            if(result[0].name != pipe_name)
                                            {
                                                throw new Error(JSON.stringify({test:"[js-test] Error (result[0].name != pipe_name) in query:"+query, error:error, result:result, fields:fields }));
                                            }

                                            if(result[0].message != text_message)
                                            {
                                                throw new Error(JSON.stringify({test:"[js-test] Error (result[0].message != text_message) in query:"+query, error:error, result:result, fields:fields }));
                                            }

                                            query = 'DELETE FROM pipes_messages WHERE name = "'+pipe_name+'" ';
                                            connection1.query(query,
                                                function(error, result, fields)
                                                {
                                                    if(error)
                                                    {
                                                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                                                    }

                                                    console.log("[js-test] \x1b[1;32m test_ql_pipes_settings 5\x1b[0m", result);
                                                    var text_message = "text message "+Math.random()
                                                    query = 'INSERT INTO pipes_messages (name, event, message)VALUES("'+pipe_name+'", "event_in_pipe", "'+text_message+'");';
                                                    connection1.query(query,
                                                        function(error, result, fields)
                                                        {
                                                            if(error)
                                                            {
                                                                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                                                            }

                                                            console.log("[js-test] \x1b[1;32m test_ql_pipes_settings 3\x1b[0m", result);
                                                            query = 'SELECT * FROM pipes_messages WHERE name = "'+pipe_name+'" ';
                                                            connection1.query(query,
                                                                function(error, result, fields)
                                                                {
                                                                    if(error)
                                                                    {
                                                                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                                                                    }

                                                                    console.log("[js-test] \x1b[1;32m test_ql_pipes_settings 4\x1b[0m", result);
                                                                    /*  Normal answer:  {
                                                                        id: '5f358dd6-6733-11e8-a',
                                                                        time: '1528033097',
                                                                        name: 'secret_pipe',
                                                                        index: '0',
                                                                        event: 'event_in_pipe',
                                                                        message: 'text message',
                                                                        user_id: '0'
                                                                    }*/

                                                                    if(result.length != 1)
                                                                    {
                                                                        throw new Error(JSON.stringify({test:"[js-test test_ql_pipes_settings 3] Error (more then one line in result) in query:"+query, error:error, result:result, fields:fields }));
                                                                    }

                                                                    if(result[0].name != pipe_name)
                                                                    {
                                                                        throw new Error(JSON.stringify({test:"[js-test] Error (result[0].name != pipe_name) in query:"+query, error:error, result:result, fields:fields }));
                                                                    }

                                                                    if(result[0].message != text_message)
                                                                    {
                                                                        throw new Error(JSON.stringify({test:"[js-test] Error (result[0].message != text_message) in query:"+query, error:error, result:result, fields:fields }));
                                                                    }

                                                                    query = 'DELETE FROM pipes_settings WHERE name = "'+pipe_name+'" ';
                                                                    connection1.query(query,
                                                                        function(error, result, fields)
                                                                        {
                                                                            if(error)
                                                                            {
                                                                                throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                                                                            }

                                                                            console.log("[js-test] \x1b[1;32m test_ql_pipes_settings 5\x1b[0m", result);
                                                                            query = 'SELECT * FROM pipes_messages WHERE name = "'+pipe_name+'" ';
                                                                            connection1.query(query,
                                                                                function(error, result, fields)
                                                                                {
                                                                                    if(error)
                                                                                    {
                                                                                        throw new Error(JSON.stringify({test:"[js-test] Error in query:"+query, error:error, result:result, fields:fields }));
                                                                                    }

                                                                                    if(result.length != 0)
                                                                                    {
                                                                                        throw new Error(JSON.stringify({test:"[js-test] Error (result.length != 0) in query:"+query, error:error, result:result, fields:fields }));
                                                                                    }
                                                                                    
                                                                                    console.log("[js-test] \x1b[1;32m test_ql_pipes_settings 7\x1b[0m");
                                                                                }
                                                                            );
                                                                        }
                                                                    );
                                                                }
                                                            );
                                                        }
                                                    );
                                                }
                                            );
                                        }
                                    );
                                }
                            );
                        }
                    );
                }
            );
        }
    );
}
/*  */
test_setUserData(9999-2, Math.random()+"-"+Math.random()+"-"+Math.random());

test_setUserData(9999-3, Math.floor(Math.random()*100));
test_onAuth();
test_web_pipe_send();
test_getTrackPipeUsers();
test_getUserData();
test_realUserKey();
test_track_subscription();
test_ql_subscription();
test_ql_pipes_settings();

test_onAuth2();
test_JWT_Auth();
test_invalid_JWT_Auth();
test_Revoked_JWT_Auth();
test_2_users_messages();
test_users_messages();
test_get_pipe_log();
test_Revoked_JWT_table();
test_short_users_messages();// */
// _cometServerApi.prototype.stop
// restart
//
// socketArray чтоб много нод было
//
//
// count_users_in_pipe + subscription
//
// get_pipe_log + subscription
// users_messages + users_time + users_auth + subscription + JWT
// delete from users_auth
//
//
// + fs
// + cluster
//
//
