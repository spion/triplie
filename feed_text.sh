# (C) Gorgi Kosev a.k.a. Spion, John Peterson. License GNU GPL 3.
o=$(java edu.stanford.nlp.process.DocumentPreprocessor 2>&1)
if [ $? -gt 0 ]; then printf "$o\nEx. export CLASSPATH=/../stanford-parser.jar\n"; exit 1; fi
if [ -n "$2" ]; then d="-d$2"; else d=; fi
java edu.stanford.nlp.process.DocumentPreprocessor "$1" -tokenizerOptions "ptb3Escaping=false"|./text2log.sh|./feedtriplie "$d" -r"::\s([0-9]+)\s(\S+)\s(\S+)[\s\:]+(.+)"