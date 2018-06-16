
# Error codes in CometQL

^ Error code      ^ Description            ^ Comment ^
| 1     | Unspecified error| Never should not occur if the received error code 1, refer to the support        |
| 2     | This feature is not implemented, but planned.        |   | 
| 3     | Error in sql query        |  | 
| 10    | The table is read-only        | It occurs when trying to insert or delete  | 
| 11    | Too large amount of data        | You have exceeded the limit on the length of the data, such as an attempt to convey the channel name longer than 64 characters|
| 12    | Invalid data format        | For example occurs when you try to transfer as the name of the channel line content not allowed characters |
| 13    | In the where section is not given a list of strings that must be selected | At this point in the sample requests must always be passed a list of values from the first column of the table. |
| 14    | Error in the enumeration list of columns in the table | |
| 15    | The object does not exist | |
| 16    | Not passed authentication | Occurs when an incorrect username and password pair|