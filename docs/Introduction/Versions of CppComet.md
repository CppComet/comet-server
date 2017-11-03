
# Versions of CppComet

There are two versions of CppComet [pensors version](https://github.com/CppComet/comet-server) and [SaaS platform](https://comet-server.com).
They are fully compatible with Api, and the only difference is when specifying the parameters for the connection.

# Connecting from the JavaScript API

When connecting from the JavaScript API to the SaaS version

```
 CometServer(). Start ({dev_id: 15, user_id: 1, user_key: "userHash"}) 
```


When connecting from the JavaScript API to the open source version

```
 CometServer (). Start ({dev_id: 0, user_id: 1, user_key: "userHash", node: "example.com"}) 
```


# Connecting from CometQL

When connecting from CometQL to SaaS version

```

$ dev_id = "15";

// It is issued in a personal account on comet-server.com
$ Dev_key = "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8";
$ link = mysqli_connect ("app.comet-server.ru", $ dev_id, $ dev_key, "CometQL_v1");

```


When connecting from CometQL to the open source version

```

$ dev_id = "root";

// Set in the comet.ini file
$ Dev_key = "lPXBFPqNg3f661JcegBY0N0dPXqUBdHXqj2cHf04PZgLHxT6z55e20ozojvMRvB8";
$ link = mysqli_connect ("example.com", $ dev_id, $ dev_key, "CometQL_v1");

```


# Sharing Security

In the SaaS version, all user data is isolated from each other based on the dev_id parameter which is specified when connecting to the server. You are using your dev_id or how you will not affect the clients that specified when you connected another dev_id.
If we talk about [open source version](https://github.com/CppComet/comet-server), then there is no division into dev_id, that is, it is calculated that there is only one user for one server instance. Therefore, you do not need to specify the dev_id parameter when connecting.