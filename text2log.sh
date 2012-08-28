read line;
i=1
maxempty=50
empty=0
while [ "1" ]
do
    if [ "$line" ]
    then
        echo :: $i blah blah : "$line"
        empty=0
    else
        empty=$(($empty + 1))
        if [ $empty -gt $maxempty ] 
        then 
            break;
        fi
    fi
    read line
    i=$(($i + 1))
done
