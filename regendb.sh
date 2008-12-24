#!/bin/bash


if [ -f "botdata/triplie.db" ]
then
	echo 'Removing old database...'
	rm botdata/triplie.db
fi
echo 'Creating database...'
make bootstrap
if [ -f "log.txt" ]
then
	echo 'Making sure feedtriplie is upto date...'
	make feedtriplie
	echo 'Running feedtriplie for DB regeneration'
	echo 'This might take a while...'
	cat log.txt | ./feedtriplie "::\s([0-9]+)\s(\S+)\s(\S+)[\s\:]+(.+)"
	echo 'All done, new triplie ready.'
fi
