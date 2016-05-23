-- --------------------------------------------------------
-- Host:                         darkspace.net
-- Server version:               5.1.61 - Source distribution
-- Server OS:                    redhat-linux-gnu
-- HeidiSQL version:             7.0.0.4140
-- Date/time:                    2012-05-18 02:22:29
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET FOREIGN_KEY_CHECKS=0 */;

-- Dumping structure for table gamecq.access
CREATE TABLE IF NOT EXISTS `access` (
  `access_id` int(10) NOT NULL AUTO_INCREMENT,
  `access_title` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`access_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.awards
CREATE TABLE IF NOT EXISTS `awards` (
  `award_id` int(11) NOT NULL AUTO_INCREMENT,
  `text` varchar(255) NOT NULL DEFAULT '',
  `img_path` varchar(255) NOT NULL DEFAULT '',
  `img_title` varchar(100) NOT NULL DEFAULT '',
  PRIMARY KEY (`award_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.award_holders
CREATE TABLE IF NOT EXISTS `award_holders` (
  `hold_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL DEFAULT '0',
  `award_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`hold_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.bank_transactions
CREATE TABLE IF NOT EXISTS `bank_transactions` (
  `id` int(8) unsigned NOT NULL AUTO_INCREMENT,
  `type` tinyint(3) unsigned DEFAULT '0',
  `clan_id` int(8) unsigned DEFAULT '0',
  `amount` int(20) unsigned DEFAULT '0',
  `auth` tinyint(3) unsigned DEFAULT '0',
  `user_id` int(8) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.banlist
CREATE TABLE IF NOT EXISTS `banlist` (
  `ban_id` int(10) NOT NULL AUTO_INCREMENT,
  `ban_userid` int(10) DEFAULT '0',
  `ban_ip` varchar(16) DEFAULT NULL,
  `ban_start` int(32) DEFAULT NULL,
  `ban_end` int(50) DEFAULT NULL,
  `ban_time_type` int(10) DEFAULT NULL,
  `ban_machine` varchar(50) DEFAULT '0',
  `ban_from` int(10) unsigned DEFAULT '0',
  `ban_why` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`ban_id`),
  KEY `ban_id` (`ban_id`),
  KEY `ban_ip` (`ban_ip`),
  KEY `ban_userid` (`ban_userid`),
  KEY `ban_start` (`ban_start`),
  KEY `ban_end` (`ban_end`),
  KEY `ban_machine` (`ban_machine`),
  KEY `ban_why` (`ban_why`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.billing
CREATE TABLE IF NOT EXISTS `billing` (
  `billing_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `active` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `name` varchar(50) DEFAULT NULL,
  `about` text NOT NULL,
  `subscribe_url` varchar(100) NOT NULL DEFAULT '',
  `session_id_var` varchar(100) NOT NULL DEFAULT '',
  PRIMARY KEY (`billing_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.catagories
CREATE TABLE IF NOT EXISTS `catagories` (
  `cat_id` int(10) NOT NULL AUTO_INCREMENT,
  `cat_title` varchar(100) DEFAULT NULL,
  `cat_order` int(10) DEFAULT NULL,
  PRIMARY KEY (`cat_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.chat
CREATE TABLE IF NOT EXISTS `chat` (
  `message_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `author` varchar(100) NOT NULL DEFAULT '',
  `author_id` int(10) unsigned NOT NULL DEFAULT '0',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  `text` blob NOT NULL,
  `recp_id` int(10) unsigned NOT NULL DEFAULT '0',
  `room_id` int(10) unsigned NOT NULL DEFAULT '0',
  `sent` tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (`message_id`),
  KEY `time` (`time`),
  KEY `recp_id` (`recp_id`),
  KEY `room_id` (`room_id`),
  KEY `sent` (`sent`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.clans
CREATE TABLE IF NOT EXISTS `clans` (
  `clan_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `founder_user_id` int(10) unsigned DEFAULT '0',
  `name` varchar(50) NOT NULL DEFAULT '',
  `long_name` varchar(50) NOT NULL DEFAULT '',
  `home` varchar(100) NOT NULL DEFAULT '',
  `closed` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `approved` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `motto` text,
  `custom_name` varchar(100) DEFAULT NULL,
  `custom_text` varchar(100) DEFAULT NULL,
  `created` int(10) unsigned DEFAULT '0',
  `forum_id` int(10) unsigned DEFAULT '0',
  `is_forum_private` tinyint(1) unsigned DEFAULT '1',
  `notify_type` varchar(20) DEFAULT 'Email',
  `leave_notify` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`clan_id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.clan_data
CREATE TABLE IF NOT EXISTS `clan_data` (
  `clan_id` int(10) unsigned NOT NULL DEFAULT '0',
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `faction_id` int(10) unsigned DEFAULT '0',
  `faction_time` int(10) unsigned DEFAULT NULL,
  `data` blob,
  `level` int(10) unsigned NOT NULL DEFAULT '0',
  `credits` int(10) unsigned NOT NULL DEFAULT '0',
  `max_members` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`clan_id`,`game_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.clan_levels
CREATE TABLE IF NOT EXISTS `clan_levels` (
  `level` int(10) unsigned NOT NULL,
  `cost` int(10) unsigned NOT NULL DEFAULT '0',
  `max_members` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`level`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.clan_members
CREATE TABLE IF NOT EXISTS `clan_members` (
  `clan_id` int(10) unsigned NOT NULL DEFAULT '0',
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `is_valid` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `is_admin` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `is_founder` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`clan_id`,`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.config
CREATE TABLE IF NOT EXISTS `config` (
  `config_id` int(10) NOT NULL AUTO_INCREMENT,
  `sitename` varchar(100) DEFAULT NULL,
  `allow_html` int(2) DEFAULT NULL,
  `allow_bbcode` int(2) DEFAULT NULL,
  `allow_sig` int(2) DEFAULT NULL,
  `allow_namechange` int(2) DEFAULT '0',
  `admin_passwd` varchar(32) DEFAULT NULL,
  `selected` int(2) NOT NULL DEFAULT '0',
  `posts_per_page` int(10) DEFAULT NULL,
  `hot_threshold` int(10) DEFAULT NULL,
  `topics_per_page` int(10) DEFAULT NULL,
  `allow_theme_create` int(10) DEFAULT NULL,
  `override_themes` int(2) DEFAULT '0',
  `email_sig` varchar(255) DEFAULT NULL,
  `email_from` varchar(100) DEFAULT NULL,
  `default_lang` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`config_id`),
  UNIQUE KEY `selected` (`selected`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.country_blacklist
CREATE TABLE IF NOT EXISTS `country_blacklist` (
  `country_code` char(2) NOT NULL DEFAULT '0',
  PRIMARY KEY (`country_code`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='List of countries to watch';

-- Data exporting was unselected.


-- Dumping structure for table gamecq.coupons
CREATE TABLE IF NOT EXISTS `coupons` (
  `coupon_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `coupon` varchar(100) NOT NULL DEFAULT '',
  `months` int(10) unsigned NOT NULL DEFAULT '0',
  `days` int(10) unsigned NOT NULL DEFAULT '0',
  `credits` int(10) unsigned NOT NULL DEFAULT '0',
  `prestige` int(10) unsigned NOT NULL DEFAULT '0',
  `created` int(10) unsigned NOT NULL DEFAULT '0',
  `author_id` int(10) unsigned NOT NULL DEFAULT '0',
  `origin_id` varchar(32) NOT NULL DEFAULT '',
  `value` int(10) NOT NULL DEFAULT '0',
  `redeem_date` int(10) unsigned NOT NULL DEFAULT '0',
  `redeemer_id` int(10) unsigned NOT NULL DEFAULT '0',
  `redeemer_ip` varchar(32) DEFAULT NULL,
  `subscription_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`coupon_id`),
  UNIQUE KEY `coupon_key` (`coupon`),
  KEY `game_id` (`game_id`),
  KEY `redeem_date` (`redeem_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.coupon_blacklist
CREATE TABLE IF NOT EXISTS `coupon_blacklist` (
  `failure_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `failure_date` int(10) unsigned NOT NULL DEFAULT '0',
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `remote_ip` varchar(32) NOT NULL DEFAULT '',
  `count` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`failure_id`),
  KEY `remote_ip` (`remote_ip`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.disallow
CREATE TABLE IF NOT EXISTS `disallow` (
  `disallow_id` int(10) NOT NULL AUTO_INCREMENT,
  `disallow_username` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`disallow_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.documents
CREATE TABLE IF NOT EXISTS `documents` (
  `doc_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `title` text NOT NULL,
  `description` text,
  `allow_user_comments` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `show_index` tinyint(1) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`doc_id`),
  KEY `game_id` (`game_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.document_chapters
CREATE TABLE IF NOT EXISTS `document_chapters` (
  `chap_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `doc_id` int(10) unsigned NOT NULL DEFAULT '0',
  `parent_id` int(10) unsigned NOT NULL DEFAULT '0',
  `is_active` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `sort_id` int(10) DEFAULT '0',
  PRIMARY KEY (`chap_id`),
  KEY `doc_id` (`doc_id`),
  KEY `parent_id` (`parent_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.document_comments
CREATE TABLE IF NOT EXISTS `document_comments` (
  `comment_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `chap_id` int(10) unsigned NOT NULL DEFAULT '0',
  `lang_id` int(10) unsigned NOT NULL DEFAULT '0',
  `author_id` int(10) unsigned NOT NULL DEFAULT '0',
  `comment` text,
  `is_approved` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`comment_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.document_text
CREATE TABLE IF NOT EXISTS `document_text` (
  `chap_id` int(10) unsigned NOT NULL DEFAULT '0',
  `lang_id` int(10) unsigned NOT NULL DEFAULT '0',
  `title` text,
  `body` text,
  PRIMARY KEY (`chap_id`,`lang_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.events
CREATE TABLE IF NOT EXISTS `events` (
  `event_id` int(10) unsigned NOT NULL DEFAULT '0',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  `title` text NOT NULL,
  `details` text NOT NULL,
  `author` varchar(50) NOT NULL DEFAULT '0',
  `author_id` int(10) unsigned NOT NULL DEFAULT '0',
  `count_id` int(10) unsigned NOT NULL DEFAULT '0',
  `end_time` int(10) unsigned NOT NULL DEFAULT '0',
  `game_id` int(10) unsigned NOT NULL DEFAULT '1',
  `is_public` tinyint(1) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`event_id`,`count_id`,`game_id`),
  KEY `time` (`time`),
  KEY `end_time` (`end_time`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.factions
CREATE TABLE IF NOT EXISTS `factions` (
  `faction_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `fid` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(50) NOT NULL DEFAULT '',
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `logo` varchar(100) DEFAULT NULL,
  `color` varchar(50) NOT NULL DEFAULT 'FF FF FF',
  `logo_small` varchar(255) DEFAULT NULL,
  `is_open` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`faction_id`),
  KEY `fid` (`fid`),
  KEY `game_id` (`game_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.faq
CREATE TABLE IF NOT EXISTS `faq` (
  `QUESTION_ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `CATEGORY_ID` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `QUESTION` tinytext NOT NULL,
  `ANSWER` text NOT NULL,
  `AUTHOR_ID` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`QUESTION_ID`,`CATEGORY_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.faq_cat
CREATE TABLE IF NOT EXISTS `faq_cat` (
  `CATEGORY_ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `GAME_ID` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `LANG_ID` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `NAME` tinytext NOT NULL,
  PRIMARY KEY (`CATEGORY_ID`,`LANG_ID`,`GAME_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.files
CREATE TABLE IF NOT EXISTS `files` (
  `file_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned DEFAULT '0',
  `url` text NOT NULL,
  `bytes` int(16) unsigned DEFAULT '0',
  `name` text NOT NULL,
  `description` text NOT NULL,
  `downloads` int(10) unsigned NOT NULL DEFAULT '0',
  `author` text NOT NULL,
  `author_id` int(10) unsigned NOT NULL DEFAULT '0',
  `cat_id` int(10) unsigned NOT NULL DEFAULT '0',
  `created` int(10) unsigned NOT NULL DEFAULT '0',
  `is_valid` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`file_id`,`cat_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.files_cat
CREATE TABLE IF NOT EXISTS `files_cat` (
  `cat_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `name` text NOT NULL,
  `allow_user_upload` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `restricted` tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (`cat_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.forums
CREATE TABLE IF NOT EXISTS `forums` (
  `forum_id` int(10) NOT NULL AUTO_INCREMENT,
  `forum_name` varchar(150) DEFAULT NULL,
  `forum_desc` text,
  `forum_access` int(10) DEFAULT '1',
  `forum_moderator` int(10) DEFAULT NULL,
  `forum_topics` int(10) NOT NULL DEFAULT '0',
  `forum_posts` int(10) NOT NULL DEFAULT '0',
  `forum_last_post_id` int(10) NOT NULL DEFAULT '0',
  `cat_id` int(10) DEFAULT NULL,
  `forum_type` int(10) DEFAULT '0',
  `archive_time` int(10) DEFAULT '0',
  `archive_forum_id` int(10) DEFAULT '0',
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`forum_id`),
  KEY `forum_last_post_id` (`forum_last_post_id`),
  KEY `cat_id` (`cat_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.forum_access
CREATE TABLE IF NOT EXISTS `forum_access` (
  `forum_id` int(10) NOT NULL DEFAULT '0',
  `user_id` int(10) NOT NULL DEFAULT '0',
  `can_post` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`forum_id`,`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.forum_mods
CREATE TABLE IF NOT EXISTS `forum_mods` (
  `forum_id` int(10) NOT NULL DEFAULT '0',
  `user_id` int(10) NOT NULL DEFAULT '0',
  KEY `forum_id` (`forum_id`),
  KEY `user_id` (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.friends
CREATE TABLE IF NOT EXISTS `friends` (
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `friend_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`,`friend_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.games
CREATE TABLE IF NOT EXISTS `games` (
  `game_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL DEFAULT '',
  `directory` varchar(100) DEFAULT NULL,
  `root_url` varchar(100) DEFAULT NULL,
  `newlogin_url` varchar(100) DEFAULT NULL,
  `home_url` varchar(100) DEFAULT NULL,
  `download_url` varchar(100) DEFAULT NULL,
  `admin_url` varchar(100) DEFAULT NULL,
  `manual_url` varchar(100) DEFAULT NULL,
  `registry` varchar(100) DEFAULT NULL,
  `address` varchar(100) DEFAULT NULL,
  `port` int(10) unsigned DEFAULT NULL,
  `valid` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `chat_room` int(10) unsigned DEFAULT '1',
  `clan_url` varchar(100) DEFAULT NULL,
  `profile_url` varchar(100) DEFAULT NULL,
  `news_url` varchar(100) DEFAULT NULL,
  `forum_url` varchar(100) DEFAULT NULL,
  `is_public` tinyint(1) unsigned DEFAULT '0',
  `is_free` tinyint(1) unsigned DEFAULT '0',
  `is_free_trial_enabled` tinyint(1) unsigned DEFAULT '0',
  `game_email` varchar(100) DEFAULT NULL,
  `support_email` varchar(100) DEFAULT NULL,
  `billing_email` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`game_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.graph
CREATE TABLE IF NOT EXISTS `graph` (
  `graph_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `value` int(10) DEFAULT '0',
  `time` int(10) unsigned DEFAULT '0',
  `cat_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`graph_id`),
  KEY `cat_id` (`cat_id`),
  KEY `time` (`time`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.graph_cat
CREATE TABLE IF NOT EXISTS `graph_cat` (
  `cat_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned DEFAULT '0',
  `name` varchar(50) DEFAULT NULL,
  `graph_sql` text,
  PRIMARY KEY (`cat_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.hardware_comments
CREATE TABLE IF NOT EXISTS `hardware_comments` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `hid` int(10) unsigned NOT NULL DEFAULT '0',
  `comment` text NOT NULL,
  `rating` int(1) unsigned NOT NULL DEFAULT '0',
  `author` text NOT NULL,
  `author_id` int(10) unsigned NOT NULL DEFAULT '0',
  `date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `version` varchar(50) NOT NULL DEFAULT 'Unknown',
  `memory` varchar(50) NOT NULL DEFAULT 'Unknown',
  `cpu` varchar(50) NOT NULL DEFAULT 'Unknown',
  `os` varchar(50) NOT NULL DEFAULT 'Unknown',
  PRIMARY KEY (`id`,`hid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.hardware_kb
CREATE TABLE IF NOT EXISTS `hardware_kb` (
  `hid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `name` text NOT NULL,
  `author_id` int(10) unsigned NOT NULL DEFAULT '0',
  `category` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`hid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.headermetafooter
CREATE TABLE IF NOT EXISTS `headermetafooter` (
  `header` text,
  `meta` text,
  `footer` text
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.ignores
CREATE TABLE IF NOT EXISTS `ignores` (
  `key_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `ignore_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`key_id`),
  KEY `ignore_id` (`ignore_id`),
  KEY `user_id` (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.installers
CREATE TABLE IF NOT EXISTS `installers` (
  `installer_id` varchar(50) NOT NULL DEFAULT '',
  `payment_percent` float NOT NULL DEFAULT '0',
  `payment_cap` float DEFAULT '0',
  PRIMARY KEY (`installer_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.ladder_fields
CREATE TABLE IF NOT EXISTS `ladder_fields` (
  `field_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(50) NOT NULL DEFAULT '0',
  `is_logged` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `is_protected` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT 'If true, then field is protected from resets.',
  PRIMARY KEY (`field_id`,`game_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.lang_setup
CREATE TABLE IF NOT EXISTS `lang_setup` (
  `LANG_ID` tinyint(3) unsigned NOT NULL AUTO_INCREMENT,
  `GAME_ID` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `IS_DEFAULT` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `IS_ONLINE` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `LANG` varchar(20) NOT NULL DEFAULT '',
  `LANG_TOKEN` varchar(20) NOT NULL DEFAULT '',
  `FLAG_AV` tinytext NOT NULL,
  `FLAG_NA` tinytext NOT NULL,
  PRIMARY KEY (`LANG_ID`,`GAME_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='available languages';

-- Data exporting was unselected.


-- Dumping structure for table gamecq.links
CREATE TABLE IF NOT EXISTS `links` (
  `link_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned DEFAULT '0',
  `title` text NOT NULL,
  `url` text NOT NULL,
  `author` text NOT NULL,
  `author_id` int(10) unsigned NOT NULL DEFAULT '0',
  `category` int(10) unsigned NOT NULL DEFAULT '0',
  `clicks` int(10) unsigned NOT NULL DEFAULT '0',
  `last_clicks` int(10) unsigned NOT NULL DEFAULT '0',
  `description` text NOT NULL,
  `is_valid` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`link_id`),
  KEY `category` (`category`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.log_text
CREATE TABLE IF NOT EXISTS `log_text` (
  `log_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `version_id` int(10) unsigned NOT NULL DEFAULT '0',
  `text` text,
  `author` varchar(50) DEFAULT NULL,
  `author_id` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`log_id`),
  KEY `version_id` (`version_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.log_version
CREATE TABLE IF NOT EXISTS `log_version` (
  `version_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(50) DEFAULT NULL,
  `status` tinyint(1) unsigned DEFAULT '0',
  `author` varchar(50) DEFAULT NULL,
  `author_id` int(10) unsigned DEFAULT '0',
  `game_id` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`version_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.mail_config
CREATE TABLE IF NOT EXISTS `mail_config` (
  `config_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `pop_server` varchar(100) DEFAULT NULL,
  `pop_port` int(10) unsigned DEFAULT '0',
  `pop_user` varchar(100) DEFAULT NULL,
  `pop_password` varchar(100) DEFAULT NULL,
  `save_incoming` tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (`config_id`),
  KEY `game_id` (`game_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='POP3 Configuration';

-- Data exporting was unselected.


-- Dumping structure for table gamecq.mail_forms
CREATE TABLE IF NOT EXISTS `mail_forms` (
  `form_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `form_key` varchar(100) NOT NULL DEFAULT '',
  `subject` varchar(100) DEFAULT NULL,
  `body` text,
  PRIMARY KEY (`form_id`),
  KEY `game_id` (`game_id`),
  KEY `form_key` (`form_key`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Pre-Formatted Mail Forms';

-- Data exporting was unselected.


-- Dumping structure for table gamecq.mail_incoming
CREATE TABLE IF NOT EXISTS `mail_incoming` (
  `mail_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `recp` varchar(100) DEFAULT NULL,
  `reply` varchar(100) DEFAULT NULL,
  `date` int(10) unsigned DEFAULT '0',
  `subject` varchar(255) DEFAULT NULL,
  `body` longtext,
  `is_read` tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (`mail_id`),
  KEY `game_id` (`game_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.mail_queue
CREATE TABLE IF NOT EXISTS `mail_queue` (
  `mailing_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `mail_sql` text,
  `subject` varchar(255) DEFAULT NULL,
  `reply` varchar(255) DEFAULT NULL,
  `body` text,
  `game_id` int(10) unsigned DEFAULT '0',
  `author_id` int(10) unsigned DEFAULT '0',
  `time_sent` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`mailing_id`),
  KEY `time_sent` (`time_sent`),
  KEY `game_id` (`game_id`),
  KEY `author_id` (`author_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Outgoing Mail Table';

-- Data exporting was unselected.


-- Dumping structure for table gamecq.news
CREATE TABLE IF NOT EXISTS `news` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `GAME_ID` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `LANG_ID` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `ACTIVE` tinyint(1) unsigned NOT NULL DEFAULT '1',
  `TITLE` varchar(70) NOT NULL DEFAULT '',
  `ARTICLE` blob NOT NULL,
  `AUTHOR` varchar(50) NOT NULL DEFAULT '',
  `EMAIL` varchar(50) NOT NULL DEFAULT '',
  `DATE` varchar(50) NOT NULL DEFAULT '',
  `INTERNAL_DATE` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `CAT_ID` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `AUTHOR_ID` int(10) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`ID`,`LANG_ID`,`GAME_ID`),
  KEY `INTERNAL_DATE` (`INTERNAL_DATE`),
  KEY `CAT_ID` (`CAT_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.news_cat
CREATE TABLE IF NOT EXISTS `news_cat` (
  `CAT_ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `GAME_ID` int(10) unsigned NOT NULL DEFAULT '0',
  `CATEGORY` varchar(30) NOT NULL DEFAULT '',
  `IMAGE_URL` tinytext,
  PRIMARY KEY (`CAT_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='News-Categories';

-- Data exporting was unselected.


-- Dumping structure for table gamecq.payment_methods
CREATE TABLE IF NOT EXISTS `payment_methods` (
  `method_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `method_name` varchar(50) DEFAULT NULL,
  `method_desc` varchar(100) DEFAULT NULL,
  `is_active` tinyint(1) NOT NULL DEFAULT '0',
  `is_default` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`method_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.polls
CREATE TABLE IF NOT EXISTS `polls` (
  `poll_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `question_text` varchar(511) DEFAULT NULL,
  `is_active` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `start_time` int(10) unsigned NOT NULL DEFAULT '0',
  `end_time` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`poll_id`),
  KEY `start_time` (`start_time`),
  KEY `end_time` (`end_time`),
  KEY `is_active` (`is_active`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.poll_choices
CREATE TABLE IF NOT EXISTS `poll_choices` (
  `choice_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `poll_id` int(10) unsigned NOT NULL DEFAULT '0',
  `choice_text` varchar(100) DEFAULT NULL,
  `votes` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`choice_id`),
  KEY `poll_id` (`poll_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.poll_voters
CREATE TABLE IF NOT EXISTS `poll_voters` (
  `poll_id` int(10) unsigned NOT NULL,
  `user_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`poll_id`,`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.posts
CREATE TABLE IF NOT EXISTS `posts` (
  `post_id` int(10) NOT NULL AUTO_INCREMENT,
  `topic_id` int(10) NOT NULL DEFAULT '0',
  `forum_id` int(10) NOT NULL DEFAULT '0',
  `poster_id` int(10) NOT NULL DEFAULT '0',
  `post_time` varchar(20) DEFAULT NULL,
  `poster_ip` varchar(16) DEFAULT NULL,
  PRIMARY KEY (`post_id`),
  KEY `post_id` (`post_id`),
  KEY `forum_id` (`forum_id`),
  KEY `topic_id` (`topic_id`),
  KEY `poster_id` (`poster_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.posts_text
CREATE TABLE IF NOT EXISTS `posts_text` (
  `post_id` int(10) NOT NULL DEFAULT '0',
  `post_text` text,
  PRIMARY KEY (`post_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.priv_msgs
CREATE TABLE IF NOT EXISTS `priv_msgs` (
  `msg_id` int(10) NOT NULL AUTO_INCREMENT,
  `from_userid` int(10) NOT NULL DEFAULT '0',
  `to_userid` int(10) NOT NULL DEFAULT '0',
  `msg_time` varchar(20) DEFAULT NULL,
  `poster_ip` varchar(16) DEFAULT NULL,
  `msg_status` int(10) DEFAULT '0',
  `msg_text` text,
  PRIMARY KEY (`msg_id`),
  KEY `msg_id` (`msg_id`),
  KEY `to_userid` (`to_userid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.products
CREATE TABLE IF NOT EXISTS `products` (
  `product_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `cat_id` int(10) unsigned NOT NULL DEFAULT '0',
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `product_desc` varchar(100) DEFAULT NULL,
  `product_details` varchar(100) DEFAULT NULL,
  `product_image_url` varchar(255) DEFAULT NULL,
  `amount` int(10) unsigned NOT NULL DEFAULT '0',
  `credits` int(10) unsigned NOT NULL DEFAULT '0',
  `item_class` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`product_id`),
  KEY `game_id` (`game_id`),
  KEY `cat_id` (`cat_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.product_cat
CREATE TABLE IF NOT EXISTS `product_cat` (
  `cat_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `cat_desc` varchar(100) DEFAULT NULL,
  `sort_order` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`cat_id`),
  KEY `game_id` (`game_id`),
  KEY `sort_order` (`sort_order`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.product_orders
CREATE TABLE IF NOT EXISTS `product_orders` (
  `order_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `product_id` int(10) unsigned NOT NULL DEFAULT '0',
  `payment_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `quantity` int(10) unsigned NOT NULL DEFAULT '0',
  `order_date` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`order_id`),
  KEY `product_id` (`product_id`),
  KEY `payment_id` (`payment_id`),
  KEY `user_id` (`user_id`),
  KEY `order_date` (`order_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.product_payments
CREATE TABLE IF NOT EXISTS `product_payments` (
  `payment_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `payment_date` int(10) unsigned NOT NULL DEFAULT '0',
  `amount` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Amount in cents',
  `completed` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `payment_method` varchar(50) NOT NULL DEFAULT 'paypal',
  `method_payment_id` varchar(50) DEFAULT NULL COMMENT 'Payment provider payment ID',
  `method_url` text COMMENT 'Payment Provider URL',
  `status` varchar(100) DEFAULT NULL COMMENT 'Current Status of this payment',
  PRIMARY KEY (`payment_id`),
  KEY `user_id` (`user_id`),
  KEY `payment_date` (`payment_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.projects
CREATE TABLE IF NOT EXISTS `projects` (
  `project_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `parent_id` int(10) unsigned NOT NULL DEFAULT '0',
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `author_user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(100) NOT NULL DEFAULT '',
  `description` text,
  PRIMARY KEY (`project_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Developer Projects';

-- Data exporting was unselected.


-- Dumping structure for table gamecq.projects_setup
CREATE TABLE IF NOT EXISTS `projects_setup` (
  `GAME_ID` tinyint(3) unsigned NOT NULL AUTO_INCREMENT,
  `NAME` varchar(50) NOT NULL DEFAULT '',
  `ROOT_URL` tinytext NOT NULL,
  PRIMARY KEY (`GAME_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='projects listing';

-- Data exporting was unselected.


-- Dumping structure for table gamecq.ranks
CREATE TABLE IF NOT EXISTS `ranks` (
  `rank_id` int(10) NOT NULL AUTO_INCREMENT,
  `rank_title` varchar(50) NOT NULL DEFAULT '',
  `rank_min` int(10) NOT NULL DEFAULT '0',
  `rank_max` int(10) NOT NULL DEFAULT '0',
  `rank_special` int(2) DEFAULT '0',
  `rank_image` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`rank_id`),
  KEY `rank_min` (`rank_min`),
  KEY `rank_max` (`rank_max`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.referrals
CREATE TABLE IF NOT EXISTS `referrals` (
  `ref_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `remote_ip` varchar(16) NOT NULL DEFAULT '',
  `referral` varchar(30) NOT NULL DEFAULT '',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  `user_id` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`ref_id`),
  KEY `remote_ip` (`remote_ip`),
  KEY `referer` (`referral`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.reports
CREATE TABLE IF NOT EXISTS `reports` (
  `report_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `author_id` int(10) unsigned NOT NULL DEFAULT '0',
  `message` text NOT NULL,
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`report_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.rooms
CREATE TABLE IF NOT EXISTS `rooms` (
  `room_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL DEFAULT '',
  `is_static` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `is_moderated` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `is_private` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `language` int(10) unsigned NOT NULL DEFAULT '0',
  `password` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`room_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.room_members
CREATE TABLE IF NOT EXISTS `room_members` (
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `room_id` int(10) unsigned NOT NULL DEFAULT '0',
  `server_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  KEY `room_id` (`room_id`),
  KEY `server_id` (`server_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.servers
CREATE TABLE IF NOT EXISTS `servers` (
  `server_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `type` int(10) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(255) NOT NULL DEFAULT '',
  `description` text NOT NULL,
  `address` varchar(255) NOT NULL DEFAULT '',
  `port` int(10) unsigned NOT NULL DEFAULT '0',
  `max_clients` int(10) unsigned NOT NULL DEFAULT '0',
  `clients` int(10) unsigned NOT NULL DEFAULT '0',
  `last_updated` int(10) unsigned DEFAULT NULL,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `short_description` text,
  `data` longtext,
  `mid` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`server_id`),
  KEY `type` (`type`),
  KEY `last_updated` (`last_updated`),
  KEY `game_id` (`game_id`),
  KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.sessions
CREATE TABLE IF NOT EXISTS `sessions` (
  `sess_id` int(10) unsigned NOT NULL DEFAULT '0',
  `user_id` int(10) NOT NULL DEFAULT '0',
  `start_time` int(10) unsigned NOT NULL DEFAULT '0',
  `remote_ip` varchar(15) NOT NULL DEFAULT '',
  `proxy_ip` varchar(15) DEFAULT NULL,
  `mid` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`sess_id`),
  KEY `sess_id` (`sess_id`),
  KEY `start_time` (`start_time`),
  KEY `remote_ip` (`remote_ip`),
  KEY `proxy_ip` (`proxy_ip`),
  KEY `user_id` (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.smiles
CREATE TABLE IF NOT EXISTS `smiles` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `code` varchar(50) DEFAULT NULL,
  `smile_url` varchar(100) DEFAULT NULL,
  `emotion` varchar(75) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.subscriptions
CREATE TABLE IF NOT EXISTS `subscriptions` (
  `subscription_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `order_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `option_id` int(10) unsigned NOT NULL DEFAULT '0',
  `active` int(1) unsigned NOT NULL DEFAULT '1',
  `subscribe_date` int(10) unsigned NOT NULL DEFAULT '0',
  `next_bill_date` int(10) unsigned NOT NULL DEFAULT '0',
  `error_date` int(10) unsigned NOT NULL DEFAULT '0',
  `error_count` int(10) unsigned NOT NULL DEFAULT '0',
  `charges` int(10) unsigned NOT NULL DEFAULT '0',
  `referal_user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `referal_order_id` int(10) unsigned NOT NULL DEFAULT '0',
  `parent_subscription_id` int(10) unsigned NOT NULL DEFAULT '0',
  `parent_option_id` int(10) unsigned NOT NULL DEFAULT '0',
  `profile_id` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`subscription_id`),
  UNIQUE KEY `user_id` (`user_id`,`game_id`),
  KEY `next_bill_date` (`next_bill_date`),
  KEY `active` (`active`),
  KEY `option_id` (`option_id`),
  KEY `profile_id` (`profile_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.subscription_events
CREATE TABLE IF NOT EXISTS `subscription_events` (
  `event_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `subscription_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `event_date` int(10) unsigned NOT NULL DEFAULT '0',
  `text` text NOT NULL,
  PRIMARY KEY (`event_id`),
  KEY `subscription_id` (`subscription_id`),
  KEY `event_date` (`event_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.subscription_options
CREATE TABLE IF NOT EXISTS `subscription_options` (
  `option_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `amount` int(10) NOT NULL DEFAULT '0',
  `months` int(10) unsigned NOT NULL DEFAULT '0',
  `credits` int(10) unsigned NOT NULL DEFAULT '0',
  `description` text NOT NULL,
  `about` text,
  `end_date` int(10) unsigned NOT NULL DEFAULT '0',
  `default_id` int(10) unsigned NOT NULL DEFAULT '0',
  `referal_credit` int(10) unsigned NOT NULL DEFAULT '7',
  `payment_method` varchar(50) DEFAULT 'paypal',
  PRIMARY KEY (`option_id`),
  KEY `end_date` (`end_date`),
  KEY `game_id` (`game_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.subscription_orders
CREATE TABLE IF NOT EXISTS `subscription_orders` (
  `order_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `subscription_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `order_date` int(10) unsigned NOT NULL DEFAULT '0',
  `amount` int(10) NOT NULL DEFAULT '0',
  `amount_charges` int(10) NOT NULL DEFAULT '0',
  `months` int(10) unsigned NOT NULL DEFAULT '0',
  `credits` int(10) unsigned NOT NULL DEFAULT '0',
  `pending` tinyint(1) unsigned NOT NULL DEFAULT '1',
  `completed` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `ref_id` varchar(50) DEFAULT NULL,
  `ref_url` text,
  `status` varchar(100) DEFAULT NULL,
  `referal_user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `referal_credit` int(10) unsigned NOT NULL DEFAULT '0',
  `payment_method` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`order_id`),
  KEY `subscription_id` (`subscription_id`),
  KEY `order_date` (`order_date`),
  KEY `ref_id` (`ref_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.themes
CREATE TABLE IF NOT EXISTS `themes` (
  `theme_id` int(10) NOT NULL AUTO_INCREMENT,
  `theme_name` varchar(35) DEFAULT NULL,
  `bgcolor` varchar(10) DEFAULT NULL,
  `textcolor` varchar(10) DEFAULT NULL,
  `color1` varchar(10) DEFAULT NULL,
  `color2` varchar(10) DEFAULT NULL,
  `table_bgcolor` varchar(10) DEFAULT NULL,
  `header_image` varchar(100) DEFAULT NULL,
  `newtopic_image` varchar(100) DEFAULT NULL,
  `reply_image` varchar(100) DEFAULT NULL,
  `linkcolor` varchar(15) DEFAULT NULL,
  `vlinkcolor` varchar(15) DEFAULT NULL,
  `theme_default` int(2) DEFAULT '0',
  `fontface` varchar(100) DEFAULT NULL,
  `fontsize1` varchar(5) DEFAULT NULL,
  `fontsize2` varchar(5) DEFAULT NULL,
  `fontsize3` varchar(5) DEFAULT NULL,
  `fontsize4` varchar(5) DEFAULT NULL,
  `tablewidth` varchar(10) DEFAULT NULL,
  `replylocked_image` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`theme_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.tickets
CREATE TABLE IF NOT EXISTS `tickets` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `ticket_id` int(20) unsigned NOT NULL DEFAULT '0',
  `game_id` int(20) unsigned NOT NULL DEFAULT '0',
  `time` int(20) unsigned NOT NULL DEFAULT '0',
  `category` varchar(30) DEFAULT NULL,
  `subject` varchar(50) DEFAULT NULL,
  `body` text,
  `user_id` int(20) unsigned NOT NULL DEFAULT '0',
  `email` varchar(100) DEFAULT NULL,
  `status` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.topics
CREATE TABLE IF NOT EXISTS `topics` (
  `topic_id` int(10) NOT NULL AUTO_INCREMENT,
  `topic_title` varchar(100) DEFAULT NULL,
  `topic_poster` int(10) DEFAULT NULL,
  `topic_time` varchar(20) DEFAULT NULL,
  `topic_views` int(10) NOT NULL DEFAULT '0',
  `topic_replies` int(10) NOT NULL DEFAULT '0',
  `topic_last_post_id` int(10) NOT NULL DEFAULT '0',
  `forum_id` int(10) NOT NULL DEFAULT '0',
  `topic_status` int(10) NOT NULL DEFAULT '0',
  `topic_notify` int(2) DEFAULT '0',
  `sticky` tinyint(4) DEFAULT '0',
  `faction` tinyint(4) DEFAULT '0',
  PRIMARY KEY (`topic_id`),
  KEY `topic_id` (`topic_id`),
  KEY `forum_id` (`forum_id`),
  KEY `topic_last_post_id` (`topic_last_post_id`),
  KEY `topic_poster` (`topic_poster`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.users
CREATE TABLE IF NOT EXISTS `users` (
  `user_id` int(10) NOT NULL AUTO_INCREMENT,
  `loginname` varchar(40) NOT NULL DEFAULT '',
  `username` varchar(40) NOT NULL DEFAULT '',
  `user_regdate` varchar(20) NOT NULL DEFAULT '',
  `user_password` varchar(32) NOT NULL DEFAULT '',
  `user_email` varchar(50) DEFAULT NULL,
  `last_user_email` varchar(50) DEFAULT NULL,
  `user_email_valid` int(1) DEFAULT '0',
  `user_icq` varchar(15) DEFAULT NULL,
  `user_website` varchar(100) DEFAULT NULL,
  `user_occ` varchar(100) DEFAULT NULL,
  `user_from` varchar(100) DEFAULT NULL,
  `user_intrest` varchar(150) DEFAULT NULL,
  `user_sig` varchar(255) DEFAULT NULL,
  `user_sig_x` int(10) unsigned DEFAULT '0',
  `user_sig_y` int(10) unsigned DEFAULT '0',
  `user_viewemail` tinyint(2) DEFAULT NULL,
  `user_theme` int(10) DEFAULT NULL,
  `user_aim` varchar(18) DEFAULT NULL,
  `user_yim` varchar(25) DEFAULT NULL,
  `user_msnm` varchar(25) DEFAULT NULL,
  `user_posts` int(10) DEFAULT '0',
  `user_attachsig` int(2) DEFAULT '0',
  `user_desmile` int(2) DEFAULT '0',
  `user_html` int(2) DEFAULT '0',
  `user_bbcode` int(2) DEFAULT '0',
  `user_rank` int(10) DEFAULT '0',
  `user_level` int(10) DEFAULT '1',
  `user_lang` varchar(255) DEFAULT NULL,
  `user_actkey` varchar(32) DEFAULT NULL,
  `user_newpasswd` varchar(32) DEFAULT NULL,
  `last_login` int(10) unsigned DEFAULT '0',
  `last_ip` varchar(64) DEFAULT NULL,
  `last_mid` varchar(50) DEFAULT NULL,
  `country` varchar(64) DEFAULT NULL,
  `country_code` char(2) DEFAULT NULL,
  `email_offline` int(10) unsigned DEFAULT '0',
  `last_offline_email` int(10) unsigned DEFAULT '0',
  `super_admin` tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (`user_id`),
  KEY `loginname` (`loginname`),
  KEY `username` (`username`),
  KEY `last_login` (`last_login`),
  KEY `last_ip` (`last_ip`),
  KEY `country_code` (`country_code`),
  KEY `country` (`country`),
  KEY `last_mid` (`last_mid`),
  KEY `email_offline` (`email_offline`),
  KEY `user_email_valid` (`user_email_valid`),
  KEY `user_email` (`user_email`),
  KEY `last_user_email` (`last_user_email`),
  KEY `user_regdate` (`user_regdate`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.user_archive
CREATE TABLE IF NOT EXISTS `user_archive` (
  `user_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(40) DEFAULT NULL,
  `user_email` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `user_email` (`user_email`),
  KEY `username` (`username`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Deleted Users';

-- Data exporting was unselected.


-- Dumping structure for table gamecq.user_cull
CREATE TABLE IF NOT EXISTS `user_cull` (
  `cull_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `warn_date` int(10) unsigned NOT NULL DEFAULT '0',
  `cull_date` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`cull_id`),
  KEY `USER_ID` (`user_id`),
  KEY `CULL_DATE` (`cull_date`),
  KEY `WARNING_DATE` (`warn_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.user_data
CREATE TABLE IF NOT EXISTS `user_data` (
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `release` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `beta` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `demo` tinyint(1) unsigned NOT NULL DEFAULT '1',
  `moderator` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `server` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `news` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `editor` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `event` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `developer` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `admin` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `super_admin` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `locked` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `free_trial` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `score` float DEFAULT '0',
  `rank` varchar(50) DEFAULT NULL,
  `widget_id` int(10) unsigned NOT NULL DEFAULT '0',
  `data` text,
  `billing_id` int(10) unsigned DEFAULT '1',
  PRIMARY KEY (`game_id`,`user_id`),
  KEY `score` (`score`),
  KEY `free_trial` (`free_trial`),
  KEY `locked` (`locked`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.user_installer
CREATE TABLE IF NOT EXISTS `user_installer` (
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `game_id` int(10) unsigned NOT NULL DEFAULT '1',
  `installer_id` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.user_ladder
CREATE TABLE IF NOT EXISTS `user_ladder` (
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `field_id` int(10) unsigned NOT NULL DEFAULT '0',
  `value` double NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`,`field_id`),
  KEY `value` (`value`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.user_ladder_log
CREATE TABLE IF NOT EXISTS `user_ladder_log` (
  `log_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `field_id` int(10) unsigned NOT NULL DEFAULT '0',
  `value` double NOT NULL DEFAULT '0',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`log_id`),
  KEY `user_id` (`user_id`),
  KEY `field_id` (`field_id`),
  KEY `time` (`time`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.user_rankings
CREATE TABLE IF NOT EXISTS `user_rankings` (
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `username` varchar(40) DEFAULT NULL,
  `userrank` varchar(40) DEFAULT NULL,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `field_id` int(10) unsigned NOT NULL DEFAULT '0',
  `rank` int(10) unsigned NOT NULL DEFAULT '0',
  `value` double NOT NULL DEFAULT '0',
  KEY `field_id` (`field_id`),
  KEY `rank` (`rank`),
  KEY `user_id` (`user_id`),
  KEY `game_id` (`game_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.user_referrals
CREATE TABLE IF NOT EXISTS `user_referrals` (
  `id` int(3) unsigned NOT NULL AUTO_INCREMENT,
  `referred_id` int(10) unsigned DEFAULT '0',
  `referrer_id` int(10) unsigned DEFAULT '0',
  `time_check` tinyint(1) unsigned DEFAULT '0',
  `rank_check` tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.user_reload
CREATE TABLE IF NOT EXISTS `user_reload` (
  `reload_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`reload_id`),
  KEY `time` (`time`),
  KEY `user_id` (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.user_sql
CREATE TABLE IF NOT EXISTS `user_sql` (
  `sql_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL DEFAULT '',
  `sql` text,
  `is_default` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`sql_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='User Search Queries';

-- Data exporting was unselected.


-- Dumping structure for table gamecq.user_status
CREATE TABLE IF NOT EXISTS `user_status` (
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `status` text,
  `real_status` text,
  `updated` int(10) unsigned DEFAULT '0',
  `server_id` int(10) unsigned NOT NULL DEFAULT '0',
  `away` tinyint(1) unsigned DEFAULT '0',
  `muted` tinyint(1) unsigned DEFAULT '0',
  `hidden` tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.user_transactions
CREATE TABLE IF NOT EXISTS `user_transactions` (
  `transaction_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `amount` float NOT NULL DEFAULT '0',
  `method` varchar(20) DEFAULT NULL,
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  `order_id` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`transaction_id`),
  KEY `TIME` (`time`),
  KEY `USER_ID` (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='User billing transactions';

-- Data exporting was unselected.


-- Dumping structure for table gamecq.watchlist
CREATE TABLE IF NOT EXISTS `watchlist` (
  `watch_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `userid` int(10) DEFAULT '0',
  `username` varchar(50) NOT NULL DEFAULT '',
  `usermachine` varchar(50) DEFAULT NULL,
  `userip` varchar(16) DEFAULT NULL,
  `addedby` int(10) DEFAULT '0',
  `addedtime` int(32) DEFAULT NULL,
  `watch_type` int(2) unsigned NOT NULL DEFAULT '0',
  `reason` text,
  `postinglink` text,
  `is_active` tinyint(1) unsigned NOT NULL DEFAULT '1',
  `banid` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`watch_id`),
  KEY `userid` (`userid`),
  KEY `ban_id` (`banid`),
  KEY `is_active` (`is_active`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.web_menu
CREATE TABLE IF NOT EXISTS `web_menu` (
  `menu_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned DEFAULT '0',
  `sort_id` int(10) unsigned DEFAULT '0',
  `text_key` varchar(100) DEFAULT NULL,
  `html` text,
  `php` text,
  `module` varchar(100) DEFAULT NULL,
  `args` varchar(255) DEFAULT NULL,
  `is_active` tinyint(1) unsigned DEFAULT '0',
  `is_visible` tinyint(1) unsigned DEFAULT '0',
  `user` tinyint(1) unsigned DEFAULT '0',
  `vistor` tinyint(1) unsigned DEFAULT '0',
  `admin` tinyint(1) unsigned DEFAULT '0',
  `moderator` tinyint(1) unsigned DEFAULT '0',
  `subscriber` tinyint(1) unsigned DEFAULT '0',
  `editor` tinyint(1) unsigned DEFAULT '0',
  `demo` tinyint(1) unsigned DEFAULT '0',
  `developer` tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (`menu_id`),
  KEY `game_id` (`game_id`),
  KEY `sort_id` (`sort_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.web_sidebox
CREATE TABLE IF NOT EXISTS `web_sidebox` (
  `box_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `sort_id` int(10) unsigned NOT NULL DEFAULT '0',
  `style_id` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `title_key` varchar(30) DEFAULT NULL,
  `html` text,
  `module` varchar(100) DEFAULT NULL,
  `args` varchar(255) DEFAULT NULL,
  `is_visible` tinyint(1) unsigned NOT NULL DEFAULT '1',
  `is_ad` tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (`box_id`),
  KEY `game_id` (`game_id`),
  KEY `box_key` (`sort_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.web_text
CREATE TABLE IF NOT EXISTS `web_text` (
  `text_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `game_id` int(10) unsigned NOT NULL DEFAULT '0',
  `lang_id` int(10) unsigned NOT NULL DEFAULT '0',
  `text_key` varchar(50) DEFAULT NULL,
  `text` text,
  PRIMARY KEY (`text_id`),
  KEY `game_id` (`game_id`),
  KEY `lang_id` (`lang_id`),
  KEY `text_key` (`text_key`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.whosonline
CREATE TABLE IF NOT EXISTS `whosonline` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `ip` varchar(255) DEFAULT NULL,
  `name` varchar(255) DEFAULT NULL,
  `count` varchar(255) DEFAULT NULL,
  `date` varchar(255) DEFAULT NULL,
  `username` varchar(40) DEFAULT NULL,
  `forum` int(10) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.widgets
CREATE TABLE IF NOT EXISTS `widgets` (
  `widget_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `server_id` int(10) unsigned NOT NULL DEFAULT '0',
  `data` blob NOT NULL,
  `received_date` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`widget_id`),
  KEY `server_id` (`server_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table gamecq.words
CREATE TABLE IF NOT EXISTS `words` (
  `word_id` int(10) NOT NULL AUTO_INCREMENT,
  `word` varchar(100) DEFAULT NULL,
  `replacement` varchar(100) DEFAULT NULL,
  `word_type` tinyint(4) NOT NULL DEFAULT '0',
  `word_size` int(11) NOT NULL DEFAULT '0',
  `reason` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`word_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Data exporting was unselected.
/*!40014 SET FOREIGN_KEY_CHECKS=1 */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
