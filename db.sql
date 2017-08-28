
--
-- Database: `comet_db`
--

-- --------------------------------------------------------

--
-- Table structure `log_event`
--

CREATE TABLE IF NOT EXISTS `log_event` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `text` varchar(250) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=18 ;

-- --------------------------------------------------------

--
-- Table structure `log_query`
--

CREATE TABLE IF NOT EXISTS `log_query` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `query` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=10 ;

-- --------------------------------------------------------

--
-- Table structure `pipes_settings`
--

CREATE TABLE IF NOT EXISTS `pipes_settings` (
  `name` char(128) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `type` char(1) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `length` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure `pipe_messages`
--

CREATE TABLE IF NOT EXISTS `pipe_messages` (
  `id` char(20) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `time` int(11) NOT NULL,
  `pipe_name` char(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `event` char(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT '',
  `message` text CHARACTER SET utf8 COLLATE utf8_bin,
  `user_id` int(9) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `time` (`time`),
  KEY `pipe_name` (`pipe_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure `users_auth`
--

CREATE TABLE IF NOT EXISTS `users_auth` (
  `user_id` int(11) NOT NULL,
  `hash` char(32) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT '',
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure `users_messages`
--

CREATE TABLE IF NOT EXISTS `users_messages` (
  `id` char(36) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `time` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `event` char(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT '',
  `message` text CHARACTER SET utf8 COLLATE utf8_bin,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure `users_time`
--

CREATE TABLE IF NOT EXISTS `users_time` (
  `user_id` int(11) NOT NULL,
  `time` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


-- --------------------------------------------------------

--
-- Table structure `options`
--

CREATE TABLE IF NOT EXISTS `options` (
  `section` varbinary(250) NOT NULL,
  `name` varbinary(250) NOT NULL,
  `value` varbinary(250) NOT NULL,
  PRIMARY KEY (`section`,`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
