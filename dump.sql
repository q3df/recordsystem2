-- phpMyAdmin SQL Dump
-- version 3.3.3
-- http://www.phpmyadmin.net
--
-- Host: 172.16.128.1
-- Erstellungszeit: 28. Juni 2019 um 00:38
-- Server Version: 5.7.23
-- PHP-Version: 5.5.37-1+deprecated+dontuse+deb.sury.org~trusty+1

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Datenbank: `q3server`
--

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_access`
--

CREATE TABLE IF NOT EXISTS `q3_access` (
  `id` int(5) NOT NULL AUTO_INCREMENT,
  `description` varchar(255) NOT NULL,
  `cmd` varchar(50) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=14 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_defrag_demos`
--

CREATE TABLE IF NOT EXISTS `q3_defrag_demos` (
  `rec_id` int(6) NOT NULL AUTO_INCREMENT,
  `filename` varchar(255) NOT NULL DEFAULT '',
  `size` int(10) NOT NULL DEFAULT '0',
  `datetime` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `counter` int(5) NOT NULL DEFAULT '0',
  PRIMARY KEY (`rec_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_defrag_forbidden_maps`
--

CREATE TABLE IF NOT EXISTS `q3_defrag_forbidden_maps` (
  `id` tinyint(4) NOT NULL AUTO_INCREMENT,
  `mapname` varchar(100) NOT NULL,
  `timeout` int(10) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `mapname` (`mapname`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Players cannot vote these maps' AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_defrag_records`
--

CREATE TABLE IF NOT EXISTS `q3_defrag_records` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `server_id` int(5) NOT NULL,
  `user_id` int(5) NOT NULL,
  `nickname` text NOT NULL,
  `map` varchar(100) NOT NULL,
  `mstime` int(10) NOT NULL,
  `physic` int(2) NOT NULL,
  `mode` tinyint(1) NOT NULL,
  `userinfostring` mediumtext NOT NULL,
  `timestamp` int(10) NOT NULL,
  `o3j_version` varchar(100) NOT NULL DEFAULT '1016',
  `defrag_version` varchar(100) NOT NULL DEFAULT '19108',
  `status` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `user_id` (`user_id`,`map`,`physic`,`mode`),
  KEY `map` (`map`),
  KEY `places` (`map`,`physic`,`mode`,`mstime`),
  KEY `mstime` (`mstime`),
  KEY `map_2` (`map`,`physic`,`mode`),
  KEY `idx` (`server_id`,`user_id`,`map`,`mstime`,`physic`,`mode`,`timestamp`,`status`),
  KEY `timestamp` (`timestamp`) USING HASH
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=513533 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_defrag_records_history`
--

CREATE TABLE IF NOT EXISTS `q3_defrag_records_history` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `server_id` int(5) NOT NULL,
  `user_id` int(5) NOT NULL,
  `nickname` text NOT NULL,
  `map` varchar(100) NOT NULL,
  `mstime` int(10) NOT NULL,
  `physic` int(2) NOT NULL,
  `mode` tinyint(1) NOT NULL,
  `userinfostring` mediumtext NOT NULL,
  `timestamp` int(10) NOT NULL,
  `o3j_version` varchar(100) NOT NULL DEFAULT '1016',
  `defrag_version` varchar(100) NOT NULL DEFAULT '19108',
  PRIMARY KEY (`id`),
  KEY `server_id` (`server_id`),
  KEY `auth` (`map`,`physic`,`mode`),
  KEY `user_id` (`user_id`),
  KEY `mstime` (`mstime`),
  KEY `user_id_2` (`user_id`,`physic`,`mode`,`map`),
  KEY `user_id_3` (`user_id`,`map`,`mstime`,`physic`,`mode`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=2203529 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_defrag_records_tmp`
--

CREATE TABLE IF NOT EXISTS `q3_defrag_records_tmp` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `server_id` int(5) NOT NULL,
  `user_id` int(5) NOT NULL,
  `nickname` text NOT NULL,
  `map` varchar(100) NOT NULL,
  `mstime` int(10) NOT NULL,
  `physic` int(2) NOT NULL,
  `mode` tinyint(1) NOT NULL,
  `userinfostring` mediumtext NOT NULL,
  `timestamp` int(10) NOT NULL,
  `o3j_version` varchar(100) NOT NULL DEFAULT '1016',
  `defrag_version` varchar(100) NOT NULL DEFAULT '19108',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=117442 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_ip_to_country`
--

CREATE TABLE IF NOT EXISTS `q3_ip_to_country` (
  `IP_from` double NOT NULL,
  `IP_to` double NOT NULL,
  `zwei` char(2) NOT NULL,
  `drei` char(3) NOT NULL,
  `name` varchar(50) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_minnie_records`
--

CREATE TABLE IF NOT EXISTS `q3_minnie_records` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `mapname` varchar(255) NOT NULL DEFAULT '',
  `playername` varchar(32) NOT NULL DEFAULT '',
  `visname` varchar(96) NOT NULL DEFAULT '',
  `rectime` int(32) unsigned NOT NULL DEFAULT '0',
  `physic` smallint(2) unsigned NOT NULL DEFAULT '0',
  `rank` int(5) NOT NULL DEFAULT '0',
  `ts` datetime NOT NULL,
  PRIMARY KEY (`id`),
  KEY `mapname` (`mapname`),
  KEY `rectime` (`rectime`),
  KEY `physic` (`physic`),
  KEY `playername` (`playername`),
  KEY `ts` (`ts`),
  KEY `rank` (`rank`),
  KEY `mapname_2` (`mapname`,`physic`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=258300 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_news`
--

CREATE TABLE IF NOT EXISTS `q3_news` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(5) NOT NULL DEFAULT '0',
  `headline` varchar(255) NOT NULL DEFAULT '',
  `content` mediumtext NOT NULL,
  `datetime` datetime NOT NULL,
  `locked` tinyint(1) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`,`datetime`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=89 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_news_comments`
--

CREATE TABLE IF NOT EXISTS `q3_news_comments` (
  `id` int(5) NOT NULL AUTO_INCREMENT,
  `news_id` int(5) NOT NULL DEFAULT '0',
  `user_id` int(5) NOT NULL DEFAULT '0',
  `comment` mediumtext NOT NULL,
  `datetime` datetime NOT NULL,
  `delete` int(1) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `news_id` (`news_id`,`user_id`,`datetime`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=2244 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_pinboard`
--

CREATE TABLE IF NOT EXISTS `q3_pinboard` (
  `id` int(5) NOT NULL AUTO_INCREMENT,
  `uid` int(5) NOT NULL,
  `message` mediumtext NOT NULL,
  `timestamp` int(12) NOT NULL DEFAULT '0',
  `del` tinyint(1) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `uid` (`uid`),
  KEY `timestamp` (`timestamp`) USING BTREE
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=18131 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_servers`
--

CREATE TABLE IF NOT EXISTS `q3_servers` (
  `id` int(5) NOT NULL AUTO_INCREMENT,
  `ip` varchar(15) NOT NULL DEFAULT '',
  `port` int(5) NOT NULL DEFAULT '0',
  `rcon` varchar(255) NOT NULL DEFAULT '',
  `name` varchar(255) NOT NULL,
  `flag` varchar(6) NOT NULL DEFAULT 'noflag',
  `del` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `comment` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=401 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_servers_access`
--

CREATE TABLE IF NOT EXISTS `q3_servers_access` (
  `user_id` int(5) NOT NULL DEFAULT '0',
  `server_id` int(5) NOT NULL DEFAULT '0',
  `access_bits` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`,`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_servers_access_log`
--

CREATE TABLE IF NOT EXISTS `q3_servers_access_log` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `user_id` int(5) NOT NULL DEFAULT '0',
  `server_id` int(5) NOT NULL DEFAULT '0',
  `access_id` int(5) NOT NULL DEFAULT '0',
  `parameter` text NOT NULL,
  `timestamp` int(10) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`),
  KEY `server_id` (`server_id`),
  KEY `access_id` (`access_id`),
  KEY `timestamp` (`timestamp`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1169 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_servers_banned_maps`
--

CREATE TABLE IF NOT EXISTS `q3_servers_banned_maps` (
  `mapname` varchar(255) NOT NULL,
  `comment` text,
  PRIMARY KEY (`mapname`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_servers_status`
--

CREATE TABLE IF NOT EXISTS `q3_servers_status` (
  `server_id` int(5) NOT NULL,
  `name_colors` varchar(100) NOT NULL,
  `map` varchar(50) NOT NULL,
  `cur_players` int(2) NOT NULL,
  `max_players` int(2) NOT NULL,
  `speedaward` int(6) NOT NULL,
  `speedaward_name` varchar(34) NOT NULL,
  `physic` int(11) NOT NULL,
  `mode` int(11) NOT NULL,
  PRIMARY KEY (`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_servers_status_players`
--

CREATE TABLE IF NOT EXISTS `q3_servers_status_players` (
  `player_id` int(5) NOT NULL,
  `server_id` int(5) NOT NULL,
  `name_colors` varchar(35) NOT NULL,
  `ping` int(3) NOT NULL,
  `country` varchar(3) NOT NULL,
  `user_id` int(5) NOT NULL,
  `spectator_name` varchar(34) NOT NULL DEFAULT '-1',
  `mstime` int(10) NOT NULL,
  `is_spectated` tinyint(1) NOT NULL,
  `has_speedaward` tinyint(1) NOT NULL,
  PRIMARY KEY (`player_id`,`server_id`,`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_type_countrys`
--

CREATE TABLE IF NOT EXISTS `q3_type_countrys` (
  `name` varchar(255) NOT NULL DEFAULT '',
  `two` varchar(3) NOT NULL DEFAULT '',
  `three` varchar(4) NOT NULL DEFAULT '',
  PRIMARY KEY (`two`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_users`
--

CREATE TABLE IF NOT EXISTS `q3_users` (
  `id` int(7) NOT NULL AUTO_INCREMENT,
  `username` varchar(255) NOT NULL DEFAULT '',
  `password` varchar(255) NOT NULL DEFAULT '',
  `email` varchar(255) NOT NULL,
  `visname` varchar(255) NOT NULL DEFAULT '',
  `strip_visname` varchar(255) NOT NULL,
  `serverid` int(5) NOT NULL,
  `last_active` int(12) NOT NULL DEFAULT '0',
  `last_login` int(12) NOT NULL DEFAULT '0',
  `elo_start_wert` int(10) NOT NULL,
  `sessionid` varchar(255) NOT NULL,
  `avatar` varchar(255) NOT NULL,
  `hardware` mediumtext NOT NULL,
  `country` varchar(3) NOT NULL DEFAULT '',
  `dateformat` varchar(255) NOT NULL,
  `timezone` int(11) NOT NULL,
  `del` tinyint(1) NOT NULL DEFAULT '0',
  `old_username` varchar(255) NOT NULL,
  `boardid` int(7) NOT NULL,
  `backend_user` tinyint(1) NOT NULL DEFAULT '0',
  `wiki_admin` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `comment` text NOT NULL,
  `newpw` varchar(255) NOT NULL,
  `newpwkey` varchar(255) NOT NULL,
  `last_change` datetime NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `strip_visname_2` (`strip_visname`),
  KEY `visname` (`visname`),
  KEY `username` (`username`),
  KEY `strip_visname` (`strip_visname`) USING BTREE
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=12007 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_users_copy`
--

CREATE TABLE IF NOT EXISTS `q3_users_copy` (
  `id` int(7) NOT NULL AUTO_INCREMENT,
  `username` varchar(255) NOT NULL DEFAULT '',
  `password` varchar(255) NOT NULL DEFAULT '',
  `email` varchar(255) NOT NULL,
  `visname` varchar(255) NOT NULL DEFAULT '',
  `strip_visname` varchar(255) NOT NULL,
  `serverid` int(5) NOT NULL,
  `last_active` int(12) NOT NULL DEFAULT '0',
  `last_login` int(12) NOT NULL DEFAULT '0',
  `elo_start_wert` int(10) NOT NULL,
  `sessionid` varchar(255) NOT NULL,
  `avatar` varchar(255) NOT NULL,
  `hardware` mediumtext NOT NULL,
  `country` varchar(3) NOT NULL DEFAULT '',
  `dateformat` varchar(255) NOT NULL,
  `timezone` int(11) NOT NULL,
  `del` tinyint(1) NOT NULL DEFAULT '0',
  `old_username` varchar(255) NOT NULL,
  `boardid` int(7) NOT NULL,
  `backend_user` tinyint(1) NOT NULL DEFAULT '0',
  `wiki_admin` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `comment` text NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `strip_visname_2` (`strip_visname`),
  KEY `visname` (`visname`),
  KEY `username` (`username`),
  KEY `strip_visname` (`strip_visname`) USING BTREE
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=9779 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_users_online`
--

CREATE TABLE IF NOT EXISTS `q3_users_online` (
  `uid` int(5) NOT NULL,
  `sessionid` varchar(255) NOT NULL,
  `user_agent` mediumtext NOT NULL,
  `request_uri` text NOT NULL,
  `timestamp` int(13) NOT NULL,
  PRIMARY KEY (`uid`,`sessionid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_users_rights`
--

CREATE TABLE IF NOT EXISTS `q3_users_rights` (
  `uid` int(5) NOT NULL DEFAULT '0',
  `can_news_add` tinyint(1) NOT NULL DEFAULT '0',
  `can_news_edit` tinyint(1) NOT NULL DEFAULT '0',
  `can_news_del` tinyint(1) NOT NULL DEFAULT '0',
  `can_news_comment_del` tinyint(1) NOT NULL DEFAULT '0',
  `can_user_add` tinyint(1) NOT NULL DEFAULT '0',
  `can_user_edit` tinyint(1) NOT NULL DEFAULT '0',
  `can_user_del` tinyint(1) NOT NULL DEFAULT '0',
  `can_add_mddrecords` tinyint(1) NOT NULL DEFAULT '0',
  `can_recs_del` tinyint(1) NOT NULL DEFAULT '0',
  `can_rights` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_users_votes`
--

CREATE TABLE IF NOT EXISTS `q3_users_votes` (
  `vote_id` int(11) NOT NULL,
  `answer_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `timestamp` int(13) NOT NULL,
  PRIMARY KEY (`vote_id`,`answer_id`,`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_votes`
--

CREATE TABLE IF NOT EXISTS `q3_votes` (
  `id` int(5) NOT NULL AUTO_INCREMENT,
  `date_from` int(15) NOT NULL,
  `date_to` int(15) NOT NULL,
  `title` varchar(255) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_votes_answer`
--

CREATE TABLE IF NOT EXISTS `q3_votes_answer` (
  `id` int(5) NOT NULL AUTO_INCREMENT,
  `vote_id` int(5) NOT NULL,
  `answer` varchar(255) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `q3_wiki_pages`
--

CREATE TABLE IF NOT EXISTS `q3_wiki_pages` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `parent_id` int(11) DEFAULT NULL,
  `name` varchar(64) NOT NULL,
  `content` longtext NOT NULL,
  `lft` int(11) NOT NULL,
  `lvl` int(11) NOT NULL,
  `rgt` int(11) NOT NULL,
  `root` int(11) DEFAULT NULL,
  `active` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `IDX_8C939A1A727ACA70` (`parent_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=197 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `tmp`
--

CREATE TABLE IF NOT EXISTS `tmp` (
  `user_id` int(11) NOT NULL DEFAULT '0',
  `email` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  `boardid` int(11) DEFAULT NULL,
  `username` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  `count_records` int(11) DEFAULT NULL,
  `count_records_history` int(11) DEFAULT NULL,
  `count_comments` int(11) DEFAULT NULL,
  `count_posts` int(11) DEFAULT NULL,
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Constraints der exportierten Tabellen
--

--
-- Constraints der Tabelle `q3_wiki_pages`
--
ALTER TABLE `q3_wiki_pages`
  ADD CONSTRAINT `FK_8C939A1A727ACA70` FOREIGN KEY (`parent_id`) REFERENCES `q3_wiki_pages` (`id`) ON DELETE SET NULL;

