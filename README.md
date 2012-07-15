INTRODUCTION
============

Triplie is an AI bot based on 2nd up to 6th order Markov model. It uses an SQLite database for storage, and can be distributed to work on multiple machines on a LAN.

Triplie creates directed graphs which are made of

1. nodes, which represent the words read from the user
2. A graph representing Markov chains of 6th order
3. links that represent the associations between words from conversations in a network based on the Hebbian rule

To respond to a user, triplie extracts keywords from the user's text, finds their most appropriate associated keywords in the Hebbian association network, and generates replies that contain the associated keywords using multiple breadth-first-search Markov chains algorithm.

For more information on installing and configuring read below or see the README wiki page.

You can join the project's IRC channel too: #triplie on irc.freenode.net


COMPILE
============

To compile, unpack and type

    make

The generated binaries will be called "triplie" and "cmdtriplie"
triplie is the IRC version and cmdtriplie is the command line version.
Additional binaries feedtriplie and worktriplie are also generated.
You don't need to use these.
Configure the bot as explained below before running!


CONFIGURATION
============

If running the bot for the first time (there is no database file)
you should type

    make bootstrap

to create the initial database file in botdata/triplie.db

(A) triplie.conf
----------------
First thing to edit is triplie.conf. Here is what it must contain:

    server irc.freenode.net 6667
    nick triplie
    ident triplie
    name Triplie Diplie
    chan #triplie #otherchan
    sleep min max
    char !

1) The first line specifies the server here. This line MUST contain 3 words:
server hostname portnumber
They must all be correct. Otherwise the bot will not connect to the
server.

2) The second line is in the format "nick yourbotsnick" If the nickname is in
use, this simple version will not try another nick. Also, the nickname
must be lowercase, letters/numbers only.

3) The third line is what will appear as the username of the bot if no identd
is installed on the machine. Same rules as for nick apply.

4) The fourth line specifies the real name of the bot.

5) The fifth line specifies the default channels. When run, triplie will ALWAYS
try to join these channels first. The channel names are separated with space.

6) Sleep specifies minimum/maximum pause time between request and answer,
in seconds. For example
sleep 2 7
will make the bot wait between 2 and 7 seconds before answering. This is
used to prevent people from flooding the bot.

7) char specifies the command char prefix. 

Note that these lines can be handled in any order, but it is important that
there are no missing parameters in any of them. Also, all the starting words
of those lines MUST be lowercase!

(B) admins.dat / ignores.dat
--------------------------

admins.dat is a simple file which lists all nick!user@hosts with admin access.
Admins can join or part the bot, get stats for the database, use op/deop/voice
and devoice commands, change a channel topic using the bot, etc.
Choose your admin hosts carefully. On Undernet I recommend using 

    *!*@youruser.users.undernet.org 
    
for better security. On freenode the unaffiliated/affiliated hostmasks are a
good choice. At the end it should look something like this:

    mynick*!*myident@myvhost.myisp.com
    *!*@myxuser.users.undernet.org
    *!*@unaffiliated/adminnickname

You can have as many lines as you want in this file, and every line should
be a hostmask like the 2 above. You can also have only 1 line.

Obviously, ignores.dat lists ignore hosts. They are in the same format.
Its recommended that triplie ignores other triplies

(C) triplie.db
---------------
This is not a configuration file. Its best left intact.

However you should check if this file is present in the botdata
directory before running the bot for the first time. If its not, type

    make bootstrap

to create it.

It contains all the data that the bot has learned so far.


COMMANDS
=============

List of triplie's commands:

1) !join #channel
- makes the bot join the specified channel. At the present moment,
the bot does not remember the channel after shutdown.

2) !part #channel

3) !quit quitmsg
- the bot will quit IRC.

3.1) !die quitmsg
- the bot qill quit IRC and the triplie process will end.

4) !op nick1 nick2 ... nick6
- ops specified nicknames in the current channel. The other commands
!deop !vo !devo work similar to !op but are for deop, voice and devoice.

5) !topic some topic
- changes the topic of the current channel. Note that some characters are
removed and the topic is converted to lowercase

6.1) >db stats
- triplie will output database statistics


NOTES
================

triplie has been tested on ubuntu linux, gentoo linux and FreeBSD. 
Please let me know if your *nix system is unable to compile the bot.

This is an beta release, it could contain bugs and have unoptimised
resource usage. I'm hoping to improve that in next versions


LICENCE & AUTHOR
================

See LICENCE and AUTHORS (if present)

Thank you for downloading it! Have fun!
Spion, FreeNode@#triplie
