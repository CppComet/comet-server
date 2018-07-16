
# Reserved channel names

It is not recommended to use in their projects type channel names:
  * "bin_*"
  * "big_*"
  * "push_*"
  * "comet_*"
  * "sys_*"
  * "self_*"
  * "trust_*"
  * "video_*"
  * "audio_*"
  * "private_*"
These names may be used to further extend functionality. And they will have what may be not the usual properties compared with other names of channels.

# Channels with special properties

There are also channels with special properties, all of them below.
  * msg - For the delivery of personal messages in accordance with the authorization data
  * user_status_* - for automatic notification about the status of JS api users
  * web_* - Channels in which you can send messages from both CometQL and from JS api
  * track_* - for automatic notification JS api about what someone has subscribed or unsubscribed from this channel
 