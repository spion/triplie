cat $1 | text2log.sh | ./feedtriplie "::\s([0-9]+)\s(\S+)\s(\S+)[\s\:]+(.+)"
