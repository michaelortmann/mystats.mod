#! /bin/bash

echo "MyStats setup script";
echo "Copyright (C) 2003  Douglas Cau <douglas@cau.se>";
echo "";

echo "Enter location of your eggdrop.conf (ie /home/me/eggdrop/eggdrop.conf)";
read CONF;

echo "";
echo "Enter MySQL host (localhost strongly recomended):";
read shost;

echo "";
echo "Enter MySQL user:";
read suser;

echo "";
echo "Enter MySQL pass:";
read spass;

echo "";
echo "Enter MySQL database:";
read sdbase;

echo "";
echo "Enter MySQL table prefix (tables will be named prefix_users ...):"
read sprefix;

echo "";
echo "Setting up "$CONF" ..."

{
echo "
##### MYSTATS #####

set shost \"$shost\"
set suser \"$suser\"
set spass \"$spass\"
set sdbase \"$sdbase\"
set sprefix \"$sprefix\"

loadmodule mystats";
} >> $CONF;

echo "Setting up MySQL tables"

{
echo "CREATE TABLE "$sprefix"_chans (
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
) TYPE=MyISAM;";

echo "CREATE TABLE "$sprefix"_hosts (
  mask char(255) NOT NULL default '',
  uid int(11) NOT NULL default '0',
  PRIMARY KEY  (mask)
) TYPE=MyISAM;";

echo "CREATE TABLE "$sprefix"_words (
  uid INT NOT NULL ,
  cid int(11) NOT NULL default '0',
  word VARCHAR( 64 ) NOT NULL ,
  count INT DEFAULT '1' NOT NULL ,
  last TIMESTAMP NOT NULL ,
  PRIMARY KEY ( uid , cid, word )
) TYPE=MyISAM;";

echo "CREATE TABLE "$sprefix"_online (
  nick char(32) NOT NULL default '',
  cid int(11) NOT NULL default '0',
  uid int(11) NOT NULL default '0',
  mode smallint(6) NOT NULL default '0',
  signon int(11) NOT NULL default '0',
  idle timestamp(14) NOT NULL
) TYPE=MyISAM;";

echo "CREATE TABLE "$sprefix"_stats (
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
  hour_5 int(11) NOT NULL default '0'
) TYPE=MyISAM;";

echo "CREATE TABLE "$sprefix"_users (
  uid int(11) NOT NULL auto_increment,
  handle char(32) NOT NULL default '',
  type smallint(6) NOT NULL default '0',
  PRIMARY KEY  (uid)
) TYPE=MyISAM;";
} | mysql --host=$shost --user=$suser --password=$spass --database=$sdbase;

echo "Done";
echo "";
echo "Please note that this script only puts the bare minimum of settings
in your eggdrop.conf. In order to enable ie word-counting consult with the
Further configuration section in the README.";

