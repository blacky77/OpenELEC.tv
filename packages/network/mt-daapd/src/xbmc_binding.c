/*
 * $Id: dynamic-art.c 386 2004-11-13 07:14:26Z rpedde $
 * Dynamically merge .jpeg data into an mp3 tag
 *
#      Copyright (C) 2009-2011 Christian Müller (blacky.cm.77@googlemail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sqlite3.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "configfile.h"
#include "err.h"
#include "playlist.h"
#include "restart.h"

#define BLKSIZE PIPE_BUF


#define CALL_SQLITE(f)                                          \
    {                                                           \
        int i;                                                  \
        i = sqlite3_ ## f;                                      \
        if (i != SQLITE_OK) {                                   \
            DPRINTF(E_INF,L_ART,"%s failed with status %d: %s\n", #f, i, sqlite3_errmsg (db)); \
            return (-1);                                           \
        }                                                       \
    }                                                           \



#define CALL_SQLITE_EXPECT(f,x)                                 \
    {                                                           \
        int i;                                                  \
        i = sqlite3_ ## f;                                      \
        if (i != SQLITE_ ## x) {                                \
             DPRINTF(E_INF,L_ART,"%s failed with status %d: %s\n", #f, i, sqlite3_errmsg (db)); \
            return (-1);                                           \
        }                                                       \
    } 

int get_xbmc_image_fd(char* song_path, char * song_name) {
	
	sqlite3 *db;	
	char xbmc_music_database[]="/storage/.xbmc/userdata/Database/MyMusic7.db";
    	char * sql;
	const unsigned char * sql_answer;
    	sqlite3_stmt * stmt;
    	int fd;

        CALL_SQLITE (open (xbmc_music_database, & db));
    	sprintf(sql,"SELECT idThumb FROM album WHERE strAlbum=\"%s/%s\"",song_path,song_name);
    	CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, & stmt, NULL));

	while (1) {
		int s;
		s = sqlite3_step (stmt);
		if (s == SQLITE_ROW) {
		    sql_answer  = sqlite3_column_text (stmt, 0);
		    printf ("%s\n", sql_answer);
		}
		else if (s == SQLITE_DONE) {
		    break;
		}
		else {
		    fprintf (stderr, "Failed.\n");
		    return (-1);
		}
	}
	
	char * one="1";
	if(strcmp((char*)sql_answer,one)==1){
		sqlite3_close(db);
		return 0;
	}
	
	sprintf(sql,"SELECT strThumb FROM thumb where idThumb=\"%s\"",sql_answer);

	while (1) {
		int s;
		s = sqlite3_step (stmt);
		if (s == SQLITE_ROW) {
		    sql_answer  = sqlite3_column_text (stmt, 0);
		    printf ("%s\n", sql_answer);
		}
		else if (s == SQLITE_DONE) {
		    break;
		}
		else {
		    fprintf (stderr, "Failed.\n");
		    return (-1);
		}
	}
	sqlite3_close(db);
    	
	fd = open((char*)sql_answer,O_RDONLY);
    	if(fd != -1) 
		DPRINTF(E_INF,L_ART,"Got image %s (fd %d)\n",(char*)sql_answer,fd);

    return fd;
}


