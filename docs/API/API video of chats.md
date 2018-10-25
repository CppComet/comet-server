
# Comet Video API - Entry

Link to [ online demo](https://codepen.io/Levhav/pen/MooOzd/) for impatient
----


It is possible to distinguish two going near organization of video of chats on a base webRTC. First it is a transmission of signal from every participant of conference to each other
![](https://comet-server.com/wiki/lib/exe/fetch.php/comet:video:full-mesh.png)

In this configuration, there is no need for a central server and it is easy to implement, but in cases where the participants are behind NAT it is still necessary to proxy the traffic through the turn server. ( [ The answer to toster about NAT overcoming](https://toster.ru/q/138501#answer_399519)  )

But there is yet a problem in that many persons in such conference not to add because the volume of traffic will increase with every new participant. Because for a conference on 5 persons you will be necessary to send the stream four by interlocutors. And to get four videos of stream from them that to see all at once.

In such situation it is possible to pass to other approach. When all subscribers send the signal on a central server, and that in turn sends one resulting video stream to every subscriber. (Just like this and Video API is realized in CppComet)

![](https://comet-server.com/wiki/lib/exe/fetch.php/comet:video:active_ring_topology.png)

The downside of such a scheme would be the need for a central server and a significant load on it.
But on the other hand, this opens a number of possibilities.

  * More participants in a conference
  * Possibility of record of conference by a server in video file 
  * Less than loading on a network at each of participants 
  * Possibility on a management audio and video by streams by facilities of medias of server (it is possible API by queries to the server to disconnect/to include video and sound for separate users, to determine who other users see, to control duration of conversation, disconnect separate subscribers and other great deal)
# Video API

In comets server integration is built-in with FreeSwitch simplifying creation of video of conferences and video of chats. (With possibility to use all of it in the mode of cluster)

There is an opportunity to commit:
  * Video/audio calls (On WebRTC)  
  * Video/ audio conferences on a few participants (On WebRTC)  
  * Video of translation for streaming one to many (On WebRTC dispatch of video from that who video a stream publishes delivery to the audience in the formats of hls and mpeg - dash time-lagged from 5 seconds (in theory minimum possible delay for a hls stream) to 20 - 40 seconds depending on configuration of repeaters and tuning of quality) 

___
 If a problem is found in the setup or in the work, or if there are gaps in the documentation, do not be shy [ to ask questions](https://gitter.im/CppComet/Lobby) and [ to write bug reports.](https://github.com/CppComet/comet-server/issues)
___


# Demo video of conferences

![](https://comet-server.com/wiki/lib/exe/fetch.php/comet:video:video-demo-4.gif)

Some more long gif animations
  * [gif demo 1](https://comet-server.com/wiki/lib/exe/fetch.php/comet:video:video-demo-1.gif)
  * [gif demo 2](https://comet-server.com/wiki/lib/exe/fetch.php/comet:video:video-demo-3.gif)
  * [gif demo 3](https://comet-server.com/wiki/lib/exe/fetch.php/comet:video:video-demo-5.gif)

[ Online demo](https://codepen.io/Levhav/pen/MooOzd/)
<html>
<p data-height="700" data-theme-id="0" data-slug-hash="MooOzd" data-default-tab="result" data-user="Levhav" data-embed-version="2" data-pen-title="CppComet video chat example" class="codepen">See the Pen <a href="https://codepen.io/Levhav/pen/MooOzd/">CppComet video chat example</a> by Trapenok Victor (<a href="https://codepen.io/Levhav">@Levhav</a>) on <a href="https://codepen.io">CodePen</a>.</p>
<script async src="https://production-assets.codepen.io/assets/embed/ei.js"></script>
</html>


# Conception of API video chats

![Comet Video API](https://comet-server.com/wiki/lib/exe/fetch.php/comet:video:video-api.gif )
  * For work of video of chats of comets a server uses possibilities of FreeSwitch simply co-operating with him on API. 
  * Comets a server is able to distribute video and audio calls on the great number of servers with FreeSwitch. 
  * Work with FreeSwitch fully hides after API comets of server.

# CometQL API

It is added the table to CometQL of conference that contains information about active conferences.
To create a new conference and add a member to it, you must complete the query

```

INSERT INTO conference (name, user_id, caller_id, message, profile)VALUES('1', 2, 2, "", "video");

```

There are parameters: 

  * name - the name of conference (consists only of numbers)  
  * of user _ id is an identifier of the user 
  * of caller _ id added to the conference is an identifier of that user a that initiates call (not required parameter)  
  * of message is an arbitrary report 
  * of profile is the mode of call the variants " of audio" are possible, " video" (In open source of version and on demand in support in a saas version it is possible to create the modes of video of conferences that to add thread-marks, show not simply talking the moment participant of conference in given, and all at once or to influence the rules of show of participants to the conference on a general canvas. All possibilities are enumerated in documentation on [mod _ conference](https://freeswitch.org/confluence/display/FREESWITCH/mod_conference))
To add another participant it is necessary to execute the same query only to pass new user _ id and old name. Accordingly creation of conference from 5 participants will demand 5 queries of insertion in the table of conference
An example can be viewed in the file [call.php](https://github.com/CppComet/video-chat-example)
 
Selecting data from the table conference

```

select * from `conference` where name = "101";

```


Deleting data from the table conference

```

delete from `conference` where name = "101" and user_id = 29130;

```



___
 In requests, the `and` operator operates in a restricted mode. It works only for tables `conference` and` conference_members` only for the query `delete` and only with the fields` name` and `user_id`

Full support for the operator and in the syntax of queries will be implemented later, because without it there are more important tasks and the work of the absence of the operator end does not interfere.

___


Request a sample of the table `conference` only show who have been sent an invitation to the conference.

```

select * from `conference` where name = "101";

```

It does not mean that all these people are still in this conference.
To know who presently speaks it is necessary to inquire to the table `conference_members`

```

select * from `conference_members`

```

Presently the table 'of conference _ members' is accessible read-only and contains such fields

  * of name - the name of conference 
  * of user _ id is an identifier of user 
  * of join _ time - the amount of seconds passing from the moment of connecting of user 
  * of last _ talking - the amount of seconds passing from that moment when he that talked that in a conference.




# JavaScript API

In js it is necessary to connect a file [cometVideoApi.js](https://github.com/CppComet/comet-server/tree/master/api) and library [JsSIP v2.0.6](https://github.com/CppComet/comet-server/blob/alfa/api/jssip.js) 

whereupon for treatment of events of incoming call, connecting and disconnecting it is necessary to execute a next code

```


// Initiate activation cometVideoApi
cometVideoApi.start({

// Callback caused before the beginning of connecting for a call
 // Assumed that in him tuning will be set for the nearest call 
// Such as well as parameters of audio _ remote, video _ local, video _ remote and maybe yet some.
 // And then the function of cometVideoApi will be caused.acceptCall (event) 
 // And if will not be caused that means we did not take a handset.
  onCall:function(callEvent)
  {
    $(".status").html("Incoming call, press answer.");
    if(!confirm("Answer a call?"))
    {
      // It was decided not to answer the call
      $(".status").html("It was decided not to answer the call.");
      $(".StartCallBtn").show();
      return
    }

    // Take the handset if you want
    cometVideoApi.acceptCall({
      // Type of call 'audio' | 'video'
      type:'video',

// Specify the target element for the video stream from the interlocutor
      video_remote: $("#video_remote")[0],

      // Specify the target element for the audio stream from the interlocutor
      audio_remote: $("#audio_remote")[0],

      // Specify the target element for the video stream from me (my own image from the camera)
      video_local: $("#video_local")[0],
    })
    $(".status").html("Waiting for a response from other conference participants ");
  },
  
  /**
   * Callback to completion callback
   * @param {object} event
   * {
* action: "", 	// Event name
    * status: "", 	 // Call completion reason
    * callInfo: {}, 	 // Information about the call
    * time: 1000 	 // Time duration of the call
    * type: "audio"   // Type of call   * }
   */
  onCallEnd:function(event)
  {
    $(".root").removeClass("incall")
    $(".StartCallBtn").show();
    $(".status").html("Call is completed ");
    log("onCallEnd", event)
  },
  
  /**
   * Callback of raising a handset by interlocutor
   * @param {object} event
   * {
   * action:"accept",       // Event name
   * callInfo:{},           // Information about the call
   * type:"audio"           // The type of call that is selected by the interlocutor
   * }
   *
   * Callback is called only once for the first responding interlocutor
   */
  onCallAccept: function(event)
  { 
    $(".status").html ("A response is received from another participant.");
    log("onAccept", event)
  },
  
  /**
   * Callback when I and my interlocutor were connected to the server
   * @param {object} event
   * {
   * action:"start",       // Event name
   * callInfo:{},          // Information about the call
   * type:"audio"          // The type of call
   * }
   */
  onCallStart: function(event)
  {
    $(".status").html("Connecting to the mediaserver is executed. Conversation began.");
    $(".root").addClass("incall")
    log("onCallStart", event)
  },
  onOut: function(event)
  {
    // Exit of participant from a conference
    $(".status").html ("One of interlocutors abandoned a conference");
    log("onOut", event)
  },
  onIn: function(event)
  {
    // Log in of participant to the conference
    $(".status").html("Another person joined the conference.");
    log("onIn", event)
  },
})

```


# Set up a backend for video calls


___
 Further instructions on tuning of comets of server and concomitant POE need only for the users of Open Source to the version, in SaaS of version all is adjusted already 
___


# Set up comet.ini

Section [freeswitch] in comet.ini lifts on the indicated port and ip address a http interface that is responsible for a mechanism to single from the freeswitch authorizing of users.

```

[freeswitch]
ip = 0.0.0.0
thread_num = 3
statistics = 10
port = 84 

```



___
 It is important that this interface must not be accessible from external to the network. Now in support of authorizing (planned in next releases) is realized and accordingly in order to avoid the loss of data of authorizing it is necessary to settle access to this port to the only trusted ip addresses with your servers of FreeSwitch. Limiting access is possible facilities of the firewall
___

Section [sip] contains the list of addresses and ports of servers of freeswitch that is ready to process video and audio calls.

```

[sip]
port = 7443
host = ecort-n1.comet.su;  
pipesalt = pipeSecretSalt

```


# Assembling of FreeSwitch from source codes with the necessary modules
It is needed to collect FreeSwitch on instruction [FreeSwitch Building from source](https://freeswitch.org/confluence/display/FREESWITCH/Debian+8+Jessie#Debian8Jessie-BuildingFromSource)

Then to plug in assembling and in an autoload next modules
  * [mod_rtmp](https://freeswitch.org/confluence/display/FREESWITCH/RTMP+Configuration+Files) - will allow to conduct translations of video of calls and conferences
  * [mod_xml_curl](https://freeswitch.org/confluence/display/FREESWITCH/mod_xml_curl) - FreeSwitch will allow to use the same these authorizing a FreeSwitch server that present in comets server in the table of users _ auth  * mod_av
  * mod_sndfile
  * mod_shell_stream 
  * mod_ilbc
  * mod_h26x
  * mod_siren
  * mod_isac

Final script for assembling with the necessary modules

```

wget -O - https://files.freeswitch.org/repo/deb/debian/freeswitch_archive_g0.pub | apt-key add -
 
echo "deb http://files.freeswitch.org/repo/deb/freeswitch-1.6/ jessie main" > /etc/apt/sources.list.d/freeswitch.list
apt-get update
apt-get install -y --force-yes freeswitch-video-deps-most
 
# because we're in a branch that will go through many rebases it's
# better to set this one, or you'll get CONFLICTS when pulling (update)
git config --global pull.rebase true
 
# then let's get the source. Use the -b flag to get a specific branch
cd /usr/src/
git clone https://freeswitch.org/stash/scm/fs/freeswitch.git -bv1.6 freeswitch
cd freeswitch
./bootstrap.sh -j

sed -e "s/#codecs\/mod_isac/codecs\/mod_isac/g" /usr/src/freeswitch/modules.conf | sed -e "s/#applications\/mod_av/applications\/mod_av/g" | sed -e "s/#endpoints\/mod_rtmp/endpoints\/mod_rtmp/g"  | sed -e "s/#xml_int\/mod_xml_curl/xml_int\/mod_xml_curl/g"  | sed -e "s/#applications\/mod_av/applications\/mod_av/g"| sed -e "s/#formats\/mod_sndfile/formats\/mod_sndfile/g"  | sed -e "s/#formats\/mod_shell_stream/formats\/mod_shell_stream/g" | sed -e "s/#codecs\/mod_ilbc/codecs\/mod_ilbc/g"| sed -e "s/#codecs\/mod_h26x/codecs\/mod_h26x/g"  | sed -e "s/#codecs\/mod_siren/codecs\/mod_siren/g"| sed -e "s/#codecs\/mod_isac/codecs\/mod_isac/g"   > /usr/src/freeswitch/modules.tmp

cat /usr/src/freeswitch/modules.tmp > /usr/src/freeswitch/modules.conf
rm -rf /usr/src/freeswitch/modules.tmp

./configure
make
make install

# create user 'freeswitch'
# add it to group 'freeswitch'
# change owner and group of the freeswitch installation
cd /usr/local
groupadd freeswitch
adduser --quiet --system --home /usr/local/freeswitch --gecos "FreeSWITCH open source softswitch" --ingroup freeswitch freeswitch --disabled-password
chown -R freeswitch:freeswitch /usr/local/freeswitch/
chmod -R ug=rwX,o= /usr/local/freeswitch/
chmod -R u=rwx,g=rx /usr/local/freeswitch/bin/*

sed -e "s/EnvironmentFile=-\/etc\/default\/freeswitch/EnvironmentFile=-\/etc\/freeswitch/g" /usr/src/freeswitch/debian/freeswitch-systemd.freeswitch.service | sed -e "s/PIDFile=\/run\/freeswitch\/freeswitch.pid/PIDFile=\/usr\/local\/freeswitch\/run\/freeswitch.pid/g" > /etc/systemd/system/freeswitch.service


cp /usr/local/freeswitch/bin/freeswitch  /usr/bin/freeswitch
mkdir /run/freeswitch

systemctl daemon-reload
systemctl enable freeswitch




```


It is then necessary to execute instructions [Debian+Post-Install+Tasks](https://freeswitch.org/confluence/display/FREESWITCH/Debian+Post-Install+Tasks)


# Plugging of the modules in an autoload # 

To plugging the startup modules, you need to make sure that the file /usr/local/freeswitch/conf/autoload_configs/modules.conf.xml has the following lines

```

<load module="mod_av"/>
<load module="mod_sndfile"/>
<load module="mod_shell_stream"/>
<load module="mod_rtmp"/>
<load module="mod_xml_curl"/>
<load module="mod_ilbc"/>
<load module="mod_h26x"/>
<load module="mod_siren"/>
<load module="mod_isac"/>

```


# Tuning xml_curl.conf on freeswitch # 
In the 
```
 /usr/local/freeswitch/conf/autoload_configs/xml_curl.conf.xml </ code> file, you need to enter the port and the address to connect to the comet server with the requisites that we specified in the section 
[freeswitch]


```

<configuration name="xml_curl.conf" description="cURL XML Gateway">
  <bindings>    
    <binding name="directory"> 
      <param name="gateway-url" value="http://app.comet-server.ru:84/directory" bindings="directory"/>  
      <param name="method" value="GET" /> 
    </binding>  
  </bindings> 
</configuration>

```


This tuning is responsible for that users that is authorized on comets server could from the same data of authorizing connected to freeswitch for the feasance of call.
Here are the details of the module work [mod_xml_curl](https://freeswitch.org/confluence/display/FREESWITCH/mod_xml_curl+PHP+example)

# Tuning conference.conf.xml on freeswitch # 

For tuning of different profiles of work of video of conferences it is necessary to edit a file /usr/local/freeswitch/conf/autoload_configs/conference.conf.xml

Documentation is on [mod_conference](https://freeswitch.org/confluence/display/FREESWITCH/mod_conference)
the default settings are suitable.
If you want to broadcast a conference of viewers on rtmp then in the settings of the conference profile you can enter the command:

```

<param name="auto-record" value="rtmp://comet.su/cam1/${conference_name}"/>

```


With the indication of your nginx with nginx-rtmp-module
# Tuning dialplan/default.xml on freeswitch # 

The /usr/local/freeswitch/conf/dialplan/default.xml file contains dialing commands. He used to determine what to do when calling for real numbers.
In the section 
```
 <context name = "default"> </ code> you need to add an extension to the video conferencing set

```
    
    <extension name="comet_conferences">
      <condition field="destination_number" expression="^([0-9]{1,9}\*[0-9A-z\-_]+)\*([0-9A-z\-_]+)$">
	    <!--
                    If you only have an audio call, you can add this line
                    <action application="set" data="absolute_codec_string=PCMU,GSM"/>
             -->
	    <action application="answer"/>
	    <action application="conference" data="$1-${domain_name}@$2"/>
      </condition>
    </extension>

```


# Tuning vars.xml on freeswitch # 

Useful in vars.xml to set a proof password in a line, he will be used comets by a server for api calls.

```
    
 <X-PRE-PROCESS cmd="set" data="default_password=1234"/>

```


It is needed to set the variable domain=FSdefaultDomain

```
    
  <X-PRE-PROCESS cmd="set" data="domain=FSdefaultDomain"/>

```


# Tuning event_socket.conf on freeswitch # 

In event_socket.conf, configure which port and with what password freeswitch will allow to connect to execute api commands.

```
    
 <configuration name="event_socket.conf" description="Socket Client">
  <settings>
    <param name="nat-map" value="false"/>
    <param name="listen-ip" value="0.0.0.0"/>
    <param name="listen-port" value="8021"/>
    <param name="password" value="MyPassword"/> 
    <param name="apply-inbound-acl" value="0.0.0.0/32"/> 
  </settings> 
</configuration>

```


# Tuning https on freeswitch # 

Usually after setting of freeswitch the self-signed certificate lies in the folder of 
```
/usr/local/freeswitch/certs/wss.pem
```
. For work of video and audio chats of them it is necessary to substitute by valid ones because differently from a https web-site will not turn out connected to freeswitch on websockets for an exchange by sip commands with freeswitch

For the receipt of certificates it comfortably to use Let's Encrypt. I use [this instruction](https://habrahabr.ru/post/318952/)
And as post - hook for updating of certificate such command:

```

cat /etc/letsencrypt/live/fs-n3.elevenow.com/fullchain.pem > /usr/local/freeswitch/certs/wss.pem && cat /etc/letsencrypt/live/fs-n3.elevenow.com/privkey.pem >> /usr/local/freeswitch/certs/wss.pem && cat /etc/letsencrypt/live/fs-n3.elevenow.com/fullchain.pem > /usr/local/freeswitch/certs/dtls-srtp.pem && cat /etc/letsencrypt/live/fs-n3.elevenow.com/privkey.pem >> /usr/local/freeswitch/certs/dtls-srtp.pem  && systemctl stop freeswitch && rm -rf /var/lib/freeswitch/db && systemctl start freeswitch

```

# Tuning broadcasts in nginx

For broadcasts, you need to compile nginx with the nginx-rtmp-module module here: [weak instruction](https://www.dmosk.ru/instruktions.php?object=nginx-rtmp#build)

Here is the  example of configuration nginx


```

  worker_processes  1;

    rtmp {

        server {
                    live on;
            listen 1935;
            chunk_size 512;
            buflen 1s;
            idle_streams off;

            application cam1 {
                live on;
                
                    record off;
                    
                            hls on;
                            hls_path /tmp/hls;
                            hls_fragment 2s;
                            hls_playlist_length 20s; 
                            hls_type live;

                            #hls_continuous on;

                            hls_fragment_naming sequential;

                            hls_nested on; 
                            
                            dash on;
                dash_path /tmp/dash;

                dash_fragment 2s;
                dash_playlist_length 10s;
                dash_nested on;
            }
        }
    }

    server {
            listen 80 default_server;
            listen [::]:80 default_server;

            root /var/www/html;

            index index.html index.htm index.nginx-debian.html;

            server_name _;

        include acme;

        location /dash {
            root /tmp;

            add_header Access-Control-Allow-Origin *;
            add_header Cache-Control no-cache;
        }

       location / {
            add_header Cache-Control no-cache;
            add_header Access-Control-Allow-Origin *;
            root /tmp;
        } 


    }


    server {
            listen 0.0.0.0:443;  
            server_name comet-server.com;

            ssl on;
            ssl_certificate /etc/letsencrypt/live/stream-n1.example.com/fullchain.pem;
            ssl_certificate_key /etc/letsencrypt/live/stream-n1.example.com/privkey.pem;

            ssl_session_timeout 70m;

            keepalive_disable none;
            lingering_close always;
            send_timeout 3600s;

            client_max_body_size 100m;

        include acme;

        location /dash {
            root /tmp;

                    add_header Access-Control-Allow-Origin *;
            add_header Cache-Control no-cache;
        }


            location / {
            add_header Cache-Control no-cache;
                    add_header Access-Control-Allow-Origin *;
            root /tmp;
        } 

    }

```


These articles can be useful
  * https://github.com/arut/nginx-rtmp-module/wiki/Directives
  * https://habrahabr.ru/post/162237/#workers
