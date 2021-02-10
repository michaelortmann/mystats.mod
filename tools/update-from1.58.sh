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
