#ifndef _SUPERDIFF_SOURCE_
#define _SUPERDIFF_SOURCE_

#include "superdiff.h"

typedef struct 
{
	char *name;
	char *dispname;
	char *data;
	int size;
} filestore_t;
enum dataformat {
	DATAFORMAT_HEX,
	DATAFORMAT_RAW
};
void redraw_all(void);

int filecount;

volatile int cursorY=0,cursorX=0,maxX,maxY,redraw=1,resize=0;
volatile struct winsize ws;

void init_colors(void);
void signal_handler( int signum );
void format_str_callback( char in, enum dataformat fmt, char *buffer );

char getbyte( filestore_t *cabinet, int y_coord, int x_coord )
{
	char mychar;
	filestore_t row;

	if( y_coord < filecount )
	{
		row = cabinet[ y_coord ];

		if( x_coord < row.size )
			mychar = row.data[ x_coord ];
		else
			mychar = '\0';
	}
	else
		mychar = '\0';

	return mychar;
}

// Viewport
int screen_width = -1;
int screen_height = -1;


int main( int argc, char *argv[] ) 
{


	int i;
	int myfilenum;

	char cmd_buffer[128];
	char *cb = cmd_buffer;
	*cb = '\0';

	int flag_short_names = 1;

	int longestsize = 0;

	filestore_t *filecab = NULL;
	filestore_t *fc = NULL;
	
	filecount = argc - 1;
	if( filecount < 1 ) {
		fprintf( stderr, "\nMust specify at least one input file. Two or more makes more sense.\n\n" );
		return -1;
	}

	filecab = (filestore_t*)malloc( sizeof(filestore_t) * filecount );
	fc = filecab;

	myfilenum = 1;
	while( myfilenum <= filecount )
	{
		char *mydata;
		int mysize;
		char *myfile = argv[myfilenum];

		mysize = read_file_into_mem( myfile, &mydata );
		if( mysize < 0 )
		{
			fprintf( stderr, "Error reading `%s'\n", myfile );
			return -1;
		}
		else
		{
			if( flag_short_names ) 
			{
				char *shortname;
				int namelen = strlen( myfile );

				shortname = (char*)malloc( sizeof(char)*(SHORT_NAME_WIDTH + 1) );
				substr( shortname, myfile, namelen-SHORT_NAME_WIDTH, namelen );

				fc->dispname = shortname;
			}
			else
			{
				fc->dispname = myfile;
			}
			fc->name = myfile;
			fc->data = mydata;
			fc->size = mysize;

			if( mysize > longestsize )
				longestsize = mysize;
		}

		fc++;
		myfilenum++;
	}

	// Start curses
	initscr();
	halfdelay(4);

	noecho();
	keypad(stdscr,TRUE);
	nodelay(stdscr,TRUE);

	if( has_colors() )
	{
		start_color();
		init_colors();
	}

	fc = filecab;
	int longestfilename = 0;
	for( int i=0; i < filecount; i++ )
	{
		int mylen = strlen( fc->dispname );
		fc++;
		if( mylen > longestfilename )
			longestfilename = mylen;
	}

	signal( SIGWINCH, signal_handler );
	signal( SIGINT, signal_handler );
	int end = 0; // set true to quit the program on next loop

	getmaxyx( stdscr, maxY, maxX );
	int input;

	// Viewport
	screen_width = maxX;
	screen_height = maxY;
	int data_offset = 0;	
	int filecab_offset = 0;
	volatile int data_width = 0;

	enum dataformat format = DATAFORMAT_RAW;

	while(1)
	{

		int bytewidth = 2;
		switch(format)
		{
			case DATAFORMAT_RAW: 
				bytewidth = 1; 
				break;
			case DATAFORMAT_HEX: 
				bytewidth = 2; 
				break;
		}

		data_width = floor(screen_width / bytewidth);

		// Calculate magic_char for each col
		char magic_chars[data_width];
		memset( magic_chars, 0, data_width * sizeof(char) );
		for( int x = 0; x < data_width; x++ )
		{
			char votes[0xff];
			memset( votes, 0, sizeof(char) * 0xff );
			for( int y = 0; y < filecount; y++ )
			{
				char byte = getbyte( filecab, y, x + data_offset );
				votes[byte] = votes[byte] + 1;
			}

			char mchar;
			char mchar2;
			char topvote = -1;
			for( int c = 0; c < 0xff; c++ )
			{
				if( votes[c] > topvote )
				{
					mchar2 = mchar;
					
					topvote = votes[c];
					mchar = (char)c;
				}
			}
			magic_chars[(unsigned char)x] = mchar;
		}

		// Print data
		int max_height = screen_height - 2;
		for( int y = 0; y < max_height; y++ )
		{
			for( int x = 0; x < data_width; x++ )
			{
				char output[10];
				if( y < filecount )
				{
					char byte = getbyte( filecab, y, x + data_offset );
					format_str_callback( byte, format, output );

					int color;
					if( (y == cursorY) || (x == cursorX) )
					{
						if( byte == magic_chars[x] )
							color = COLOR_SAME_CURSOR;
						else
							color = COLOR_DIFF_CURSOR;
					}
					else
					{
						if( byte == magic_chars[x] )
							color = COLOR_SAME;
						else
							color = COLOR_DIFF;
					}
						
					attron( COLOR_PAIR(color) );
					mvprintw( y-filecab_offset, (x*bytewidth), "%s", output );
					attroff( COLOR_PAIR(color) );
				}
				else
				{
					attron( COLOR_PAIR(0) );
					format_str_callback( 0, format, output );
					mvprintw( y-filecab_offset, (x*bytewidth), "%s", output );
					attroff( COLOR_PAIR(0) );
				}
			}
		}
		attroff( COLOR_PAIR(0) );

		
		input = getch();
		if( input != ERR )
		{
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
			int move = 0;
			switch(input) 
			{
				case 'h': //left
				case KEY_LEFT:
					move = LEFT;
					break;
				case 'l': //right
				case KEY_RIGHT:
					move = RIGHT;
					break;
				case 'j': //down
				case KEY_DOWN:
					move = DOWN;
					break;
				case 'k': //up
				case KEY_UP:
					move = UP;
					break;
				
				case 'q':
					end = 1;
					break;
				case 0x1b: // ESC
					cmd_buffer[0] = '\0';
					redraw = 1;
					break;
				default:
					if( isprint( input ) ) {
						sprintf( cmd_buffer, "%s%c", cmd_buffer, input );
						redraw = 1;
					}
					break;
			}
			
			if(move)
			{
				int multiply = 1;
				if( strlen( cmd_buffer ) > 0 )
				{
					multiply = atoi( cmd_buffer );
					if( multiply == 0 )
						multiply = 1;
					cmd_buffer[0] = '\0';
				}
				switch( move )
				{
					case LEFT:
						cursorX -= multiply;
						break;
					case RIGHT:
						cursorX += multiply;
						break;
					case DOWN:
						cursorY += multiply;
						break;
					case UP:
						cursorY -= multiply;
						break;
				}

				int min,max;
				min = data_offset - 1;
				
				if( cursorX < 0 )
				{
					data_offset += cursorX;
					cursorX = 0;
				}
				else if( cursorX >= data_width )
				{
					data_offset += cursorX - data_width + 1;
					cursorX = data_width - 1;
				}
				redraw = 1;
			}

//			if( cursorY >= (max_height + filecab_offset) )
//				filecab_offset++;
			if( cursorY < 0 )
				cursorY = 0;
//			if( cursorY < filecab_offset )
//				filecab_offset = cursorY;

//			if( filecab_offset + max_height > filecount )
//				filecab_offset--;
//
//			if(filecab_offset<0)
//				filecab_offset = 0;

			if( (data_offset + data_width) >= longestsize )
				data_offset = longestsize - data_width - 1;
			if( data_offset < 0 )
				data_offset = 0;
		}
			
		mvprintw( maxY-1, 0, "CURSOR: %d,%d           ", cursorX, cursorY );
		mvprintw( maxY-2, 0, "FILECAB_OFFSET: %5d", filecab_offset );
		refresh();

		continue;


		if( redraw )
		{
			i = cursorY;
			for( int j=0; j < data_width; j++ ) 
			{
				char thischar;
				int color;
				color = COLOR_SAME;
				if( cursorX == j )
					color = COLOR_SAME_CURSOR;

				/* Compare to the file at the cursor's position */
				char magic_char = 0;
				if( (data_offset-1+j) < filecab[cursorY].size )
				{
					magic_char = filecab[cursorY].data[data_offset-1+j];
				}

				if( (data_offset-1+j) < fc->size )
				{

					thischar = getbyte( filecab, i, j );
					if( thischar != magic_char ) {
						if( cursorX == j )
							color = COLOR_DIFF_CURSOR;
						else
							color = COLOR_DIFF;

					}

					if( !isprint(thischar) )
						thischar = '.';
				}
				else
					thischar = ' ';

				if( cursorY == i )
				{
					attron( A_BOLD );
					if( cursorX == j )
						color = COLOR_CROSSHAIR;
					else
						color = COLOR_SAME_CURSOR;
				}
				attron( COLOR_PAIR(color) );
				mvprintw( i, longestfilename + 1 + j, "%c", thischar );
				attroff( COLOR_PAIR(color) );
				if( cursorY == i ) 
				{
					attroff( A_BOLD );
				}
			}

			fc++;


			mvprintw( maxY-1, 0, "FILE: %s OFFSET: %d BYTE: 0x%02x(%c)",
					filecab[cursorY].name,
					data_offset + cursorX,
					(unsigned char)filecab[cursorY].data[cursorX+data_offset-1],
					(isprint(filecab[cursorY].data[cursorX+data_offset]))?filecab[cursorY].data[cursorX+data_offset-1]:'.'
					);

			int cmd_length = strlen(cmd_buffer);
			mvprintw( maxY-1, maxX - 1 - cmd_length, "%s", cmd_buffer );

			redraw = 0;

			refresh();
		}
		if( resize )
		{
			clear();
			getmaxyx( stdscr,maxY,maxX );
			fc = filecab;
			for( i=0; i < filecount; i++ )
			{
				mvprintw( i, 0, "%s", fc->dispname );
				fc++;
			}
			refresh();

			data_width = maxX - longestfilename - 1;

			fc = filecab;
			for( i=0; i < filecount; i++ )
			{
				for( int j=0; j < data_width; j++ ) 
				{
					char thischar;
					int color;
					color = COLOR_SAME;
					if( cursorX == j )
						color = COLOR_SAME_CURSOR;

					/* Compare to the file at the cursor's position */
					char magic_char = 0;
					if( (data_offset-1+j) < filecab[cursorY].size )
					{
						magic_char = filecab[cursorY].data[data_offset-1+j];
					}

					if( (data_offset-1+j) < fc->size )
					{

						thischar = getbyte( filecab, i, j );
						//thischar = fc->data[data_offset-1+j];
						
						if( thischar != magic_char ) {
							if( cursorX == j )
								color = COLOR_DIFF_CURSOR;
							else
								color = COLOR_DIFF;

						}

						if( !isprint(thischar) )
							thischar = '.';
					}
					else
						thischar = ' ';
	
					if( cursorY == i )
					{
						attron( A_BOLD );
						if( cursorX == j )
							color = COLOR_CROSSHAIR;
						else
							color = COLOR_SAME_CURSOR;
					}
					attron( COLOR_PAIR(color) );
					mvprintw( i, longestfilename + 1 + j, "%c", thischar );
					attroff( COLOR_PAIR(color) );
					if( cursorY == i ) 
					{
						attroff( A_BOLD );
					}
				}
				fc++;
			}
			refresh();

			mvprintw( maxY-1, 0, "FILE: %s OFFSET: %d BYTE: 0x%02x(%c)",
					filecab[cursorY].name,
					data_offset + cursorX,
					(unsigned char)filecab[cursorY].data[cursorX+data_offset-1],
					(isprint(filecab[cursorY].data[cursorX+data_offset]))?filecab[cursorY].data[cursorX+data_offset-1]:'.'
					);
			int cmd_length = strlen(cmd_buffer);
			mvprintw( maxY-1, maxX - 1 - cmd_length, "%s", cmd_buffer );
			refresh();

			resize = 0;
		}

		if(end)
			break;
	}

	endwin();
	return 0;
}

void format_str_callback( char in, enum dataformat fmt, char *buffer )
{
	switch(fmt)
	{
		case DATAFORMAT_RAW:
			if( isprint(in) )
			{
				sprintf( buffer, "%c", in );
			}
			else
			{
				strcpy( buffer, "." );
			}
			break;
		case DATAFORMAT_HEX:
			sprintf( buffer, "%02x", (unsigned char)in );
			break;
	}
	return;
}

void signal_handler( int signum )
{
	switch(signum)
	{
		case SIGWINCH:
			resize = 1;
			int wd = open( "/dev/tty", O_RDWR );
			
			if( ioctl( wd, TIOCGWINSZ, &ws ) != 0 ) {
				perror("ioctl(/dev/tty,TIOCGWINSZ)");
				exit(-1);
			}

			screen_width = ws.ws_col;
			screen_height = ws.ws_row;
			
			maxX=ws.ws_col;
			maxY=ws.ws_row;

			resizeterm( maxY, maxX );
			break;
		case SIGINT:
			endwin();
			exit(-1);
			break;
	}
	return;
}

void init_colors(void) 
{
	init_pair( 0, COLOR_WHITE, COLOR_BLACK );
	init_pair( COLOR_SAME, COLOR_WHITE, COLOR_BLACK );
	init_pair( COLOR_SAME2, COLOR_GREEN, COLOR_BLACK );
	init_pair( COLOR_SAME_CURSOR, COLOR_BLACK, COLOR_WHITE );
	init_pair( COLOR_SAME_CURSOR2, COLOR_GREEN, COLOR_WHITE );
	init_pair( COLOR_DIFF, COLOR_RED, COLOR_BLACK );
	init_pair( COLOR_DIFF_CURSOR, COLOR_WHITE, COLOR_RED );
	init_pair( COLOR_CROSSHAIR, COLOR_WHITE, COLOR_BLUE );
	return;
}

#endif
