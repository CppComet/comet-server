-- phpMyAdmin SQL Dump
-- version 4.6.6deb4
-- https://www.phpmyadmin.net/
--
-- : localhost:3306
--  :  17 2018 ., 02:51
--  : 10.3.7-MariaDB-1:10.3.7+maria~stretch-log
--  PHP: 7.0.27-0+deb9u1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

--
--  : `comet_db`
--

-- --------------------------------------------------------

--
--   `conference`
--

CREATE TABLE `conference` (
  `dev_id` int(11) NOT NULL,
  `name` varchar(32) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `user_id` int(11) NOT NULL,
  `caller_id` int(11) NOT NULL,
  `message` text CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `profile` varchar(32) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT '',
  `stream` varchar(255) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT '',
  `node` varchar(64) NOT NULL DEFAULT '0',
  `time` int(13) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='      ';

-- --------------------------------------------------------

--
--   `dev_config`
--

CREATE TABLE `dev_config` (
  `id` int(11) NOT NULL,
  `key` char(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `url` char(255) CHARACTER SET ascii NOT NULL DEFAULT '*',
  `active_time` int(11) NOT NULL DEFAULT 0 COMMENT '  '
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
--   `log_event`
--

CREATE TABLE `log_event` (
  `id` int(11) NOT NULL,
  `text` varchar(250) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
--   `log_query`
--

CREATE TABLE `log_query` (
  `id` int(11) NOT NULL,
  `dev_id` int(11) NOT NULL,
  `query` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
--   `pipes_settings`
--

CREATE TABLE `pipes_settings` (
  `dev_id` int(11) NOT NULL,
  `name` char(128) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `type` char(1) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `length` int(6) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
--   `pipe_messages`
--

CREATE TABLE `pipe_messages` (
  `id` char(36) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `time` int(11) NOT NULL,
  `dev_id` int(11) NOT NULL,
  `pipe_name` char(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `event` char(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT '',
  `message` text CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `user_id` int(9) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
--   `revoked_tokens`
--

CREATE TABLE `revoked_tokens` (
  `dev_id` int(11) NOT NULL,
  `token` varbinary(600) NOT NULL,
  `time` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=ascii COLLATE=ascii_bin;

-- --------------------------------------------------------

--
--   `users_auth`
--

CREATE TABLE `users_auth` (
  `dev_id` int(9) NOT NULL,
  `user_id` int(11) NOT NULL,
  `hash` char(32) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
--   `users_data`
--

CREATE TABLE `users_data` (
  `dev_id` int(9) NOT NULL,
  `user_id` int(11) NOT NULL,
  `data` varbinary(600) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
--   `users_messages`
--

CREATE TABLE `users_messages` (
  `id` char(36) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `time` int(11) NOT NULL,
  `dev_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `event` char(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT '',
  `message` text CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
--   `users_time`
--

CREATE TABLE `users_time` (
  `dev_id` int(9) NOT NULL,
  `user_id` int(11) NOT NULL,
  `time` int(11) NOT NULL DEFAULT 0
) ENGINE=MEMORY DEFAULT CHARSET=utf8;

--
--  ё 
--

--
--   `conference`
--
ALTER TABLE `conference`
  ADD PRIMARY KEY (`dev_id`,`name`,`user_id`);

--
--   `dev_config`
--
ALTER TABLE `dev_config`
  ADD PRIMARY KEY (`id`),
  ADD KEY `key` (`key`);

--
--   `log_event`
--
ALTER TABLE `log_event`
  ADD PRIMARY KEY (`id`);

--
--   `log_query`
--
ALTER TABLE `log_query`
  ADD PRIMARY KEY (`id`);

--
--   `pipes_settings`
--
ALTER TABLE `pipes_settings`
  ADD PRIMARY KEY (`dev_id`,`name`),
  ADD KEY `name` (`name`),
  ADD KEY `dev_id` (`dev_id`);

--
--   `pipe_messages`
--
ALTER TABLE `pipe_messages`
  ADD PRIMARY KEY (`id`),
  ADD KEY `time` (`time`),
  ADD KEY `pipe_name` (`pipe_name`),
  ADD KEY `dev_id` (`dev_id`);

--
--   `revoked_tokens`
--
ALTER TABLE `revoked_tokens`
  ADD PRIMARY KEY (`dev_id`,`token`);

--
--   `users_auth`
--
ALTER TABLE `users_auth`
  ADD PRIMARY KEY (`dev_id`,`user_id`),
  ADD KEY `dev_id` (`dev_id`),
  ADD KEY `user_id` (`user_id`);

--
--   `users_data`
--
ALTER TABLE `users_data`
  ADD PRIMARY KEY (`dev_id`,`user_id`),
  ADD KEY `dev_id` (`dev_id`),
  ADD KEY `user_id` (`user_id`);

--
--   `users_messages`
--
ALTER TABLE `users_messages`
  ADD PRIMARY KEY (`id`),
  ADD KEY `dev_id` (`dev_id`),
  ADD KEY `user_id` (`user_id`),
  ADD KEY `dev_id_2` (`dev_id`,`user_id`),
  ADD KEY `time` (`time`);

--
--   `users_time`
--
ALTER TABLE `users_time`
  ADD PRIMARY KEY (`dev_id`,`user_id`),
  ADD KEY `dev_id` (`dev_id`),
  ADD KEY `user_id` (`user_id`);
 