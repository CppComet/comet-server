<?php

class timeTest{

    public static $timeTestArray = array();
    public static function test($group, $name = '')
    {
        if(!isset(self::$timeTestArray[$group]))
        {
            self::$timeTestArray[$group] = array();
            self::$timeTestArray[$group][] = array("now" =>microtime(true), "name" => $name, "delta" => 0, "time" => 0 );
            return self::$timeTestArray[$group];
        }

        self::$timeTestArray[$group][] = array(
            "now" =>microtime(true),
            "name" => $name,
            "time" => microtime(true) - self::$timeTestArray[$group][0]["now"], // Общее время на группу
            "delta" => microtime(true) - self::$timeTestArray[$group][count(self::$timeTestArray[$group])-1]["now"] // Время на последнеею операцию
                );
        return self::$timeTestArray[$group];
    }
}
