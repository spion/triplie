sqlite3 triplie.db "select word from dict where id in (select distinct(id2) from assoc where id1 in (select id from dict where word = '$1') order by val desc limit 0,40);"

