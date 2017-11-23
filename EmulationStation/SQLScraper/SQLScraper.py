#!/usr/bin/env python
# -*- coding: utf-8 -*-

import xmltodict
import sqlite3
import sys


print(sys.argv[1])
systemid = sys.argv[1]

conn = sqlite3.connect("gamelist.db")

c = conn.cursor()

c.execute("delete from files where systemid = '%s'" % systemid)

c.execute("INSERT INTO files (fileid, systemid, filetype, fileexists, name, desc, image, thumbnail, rating, releasedate, developer, publisher, genre, players, playcount, lastplayed) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", ('.', systemid, '2', '1', '.', None, None, None, '0.0', None, 'unknow', 'unknow', 'unknow', 1, 0, None))


with open("%s/gamelist.xml" % systemid, encoding="utf8") as fd:
    obj = xmltodict.parse(fd.read())

        
    for row in obj["gameList"]["game"]:
        
        filetype = "1"
        fileexists = 1
        
        #    c.execute("INSERT INTO stocks ? VALUES '?'", [column["@name"], column["#text"]])
        
        fileid =  name = row.get("path", "")
        name = row.get("name", "")
        desc = row.get("desc", None)
        image = row.get("image", None)
        thumbnail = row.get("thumbnail", None)
        rating = row.get("rating", "0.0")
        releasedate = None #row.get("releasedate", "")
        developer = row.get("developer", None)
        publisher = row.get("publisher", None)
        genre = row.get("genre", None)
        players = row.get("players", "1")
        playcount = row.get("playcount", "0")
        lastplayed = row.get("lastplayed", None)
        
        #for column in row["column"]:
        c.execute("INSERT INTO files (fileid, systemid, filetype, fileexists, name, desc, image, thumbnail, rating, releasedate, developer, publisher, genre, players, playcount, lastplayed) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", (fileid, systemid, filetype, fileexists, name, desc, image, thumbnail, rating, releasedate, developer, publisher, genre, players, playcount, lastplayed))
        #print "item inserted \n"
        
conn.commit()
conn.close()        