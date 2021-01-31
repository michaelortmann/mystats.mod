-- mystats.sql  -- part of mystats.mod
-- Copyright (C) 2003  Douglas Cau <douglas@cau.se>
--
--  $Id: mystats.sql 68 2004-03-24 17:04:44Z cau $
--
-- Please try the setup.sh script before using the hard
-- way of setting the mySQL tables up. This sql file will
-- create tables with the prefix `mystats`.

-- --------------------------------------------------------

--
-- Table structure for table `mystats_chans`
--

CREATE TABLE mystats_chans (
  cid int(11) NOT NULL auto_increment,
  name varchar(128) NOT NULL default '',
  status smallint(6) NOT NULL default '-1',
  topic text NOT NULL,
  setby varchar(32) NOT NULL default '',
  seton int(11) NOT NULL default '0',
  hour_0 int(11) NOT NULL default '0',
  hour_1 int(11) NOT NULL default '0',
  hour_2 int(11) NOT NULL default '0',
  hour_3 int(11) NOT NULL default '0',
  hour_4 int(11) NOT NULL default '0',
  hour_5 int(11) NOT NULL default '0',
  PRIMARY KEY  (cid)
) TYPE=MyISAM;

-- --------------------------------------------------------

--
-- Table structure for table `mystats_hosts`
--

CREATE TABLE mystats_hosts (
  mask char(255) NOT NULL default '',
  uid int(11) NOT NULL default '0',
  PRIMARY KEY  (mask)
) TYPE=MyISAM;

-- --------------------------------------------------------

--
-- Table structure for table `mystats_online`
--

CREATE TABLE mystats_online (
  nick char(32) NOT NULL default '',
  cid int(11) NOT NULL default '0',
  uid int(11) NOT NULL default '0',
  mode smallint(6) NOT NULL default '0',
  signon int(11) NOT NULL default '0',
  idle timestamp(14) NOT NULL,
  PRIMARY KEY  (nick,cid)
) TYPE=MyISAM;

-- --------------------------------------------------------

--
-- Table structure for table `mystats_stats`
--

CREATE TABLE mystats_stats (
  uid int(11) NOT NULL default '0',
  cid int(11) NOT NULL default '0',
  line int(11) NOT NULL default '0',
  words int(11) NOT NULL default '0',
  joins int(11) NOT NULL default '0',
  kicks int(11) NOT NULL default '0',
  kicked int(11) NOT NULL default '0',
  questions int(11) NOT NULL default '0',
  topic int(11) NOT NULL default '0',
  actions int(11) NOT NULL default '0',
  online int(11) NOT NULL default '0',
  seen int(11) NOT NULL default '0',
  hour_0 int(11) NOT NULL default '0',
  hour_1 int(11) NOT NULL default '0',
  hour_2 int(11) NOT NULL default '0',
  hour_3 int(11) NOT NULL default '0',
  hour_4 int(11) NOT NULL default '0',
  hour_5 int(11) NOT NULL default '0',
  PRIMARY KEY  (uid,cid)
) TYPE=MyISAM;

-- --------------------------------------------------------

--
-- Table structure for table `mystats_users`
--

CREATE TABLE mystats_users (
  uid int(11) NOT NULL auto_increment,
  handle char(32) NOT NULL default '',
  type smallint(6) NOT NULL default '0',
  PRIMARY KEY  (uid)
) TYPE=MyISAM;

-- --------------------------------------------------------

--
-- Table structure for table `mystats_words`
--

CREATE TABLE mystats_words (
  uid int(11) NOT NULL default '0',
  cid int(11) NOT NULL default '0',
  word varchar(64) NOT NULL default '',
  count int(11) NOT NULL default '1',
  last timestamp(14) NOT NULL,
  PRIMARY KEY  (uid,cid,word)
) TYPE=MyISAM;
    
