#! /bin/bash

echo "MyStats update script";
echo "Copyright (C) 2003  Douglas Cau <douglas@cau.se>";
echo "";

echo "";
echo "Enter MySQL host (localhost strongly recommended):";
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
echo "Enter MySQL table prefix:"
read sprefix;

{
echo "ALTER TABLE "$sprefix"_chans ADD hour_0 INT DEFAULT '0' NOT NULL ;
  ALTER TABLE "$sprefix"_chans ADD hour_1 INT DEFAULT '0' NOT NULL ;
  ALTER TABLE "$sprefix"_chans ADD hour_2 INT DEFAULT '0' NOT NULL ;
  ALTER TABLE "$sprefix"_chans ADD hour_3 INT DEFAULT '0' NOT NULL ;
  ALTER TABLE "$sprefix"_chans ADD hour_4 INT DEFAULT '0' NOT NULL ;
  ALTER TABLE "$sprefix"_chans ADD hour_5 INT DEFAULT '0' NOT NULL ;";
echo "ALTER TABLE "$sprefix"_chans ADD hour_0 INT DEFAULT '0' NOT NULL ;
  ALTER TABLE "$sprefix"_stats ADD hour_1 INT DEFAULT '0' NOT NULL ;
  ALTER TABLE "$sprefix"_stats ADD hour_2 INT DEFAULT '0' NOT NULL ;
  ALTER TABLE "$sprefix"_stats ADD hour_3 INT DEFAULT '0' NOT NULL ;
  ALTER TABLE "$sprefix"_stats ADD hour_4 INT DEFAULT '0' NOT NULL ;
  ALTER TABLE "$sprefix"_stats ADD hour_5 INT DEFAULT '0' NOT NULL ;";

echo "CREATE TABLE "$sprefix"_words (
	uid INT NOT NULL ,
    cid int(11) NOT NULL default '0',
	word VARCHAR( 64 ) NOT NULL ,
	count INT DEFAULT '1' NOT NULL ,
	last TIMESTAMP NOT NULL ,
	PRIMARY KEY ( uid , cid, word )
) TYPE=MyISAM;";
} | mysql --host=$shost --user=$suser --password=$spass --database=$sdbase;

echo "Done";

