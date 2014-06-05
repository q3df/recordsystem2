#include "modules.h"

#ifdef WIN32
extern "C" {
#endif

extern Function *global;
static ModuleInfo ModInfo;

void rs_cmd_top(rs_client * cl);

C_DLLEXPORT ModuleInfo *modules_start(Function * f)
{
	global = f;
	snprintf(ModInfo.name, sizeof(ModInfo.name), "top");
	snprintf(ModInfo.author, sizeof(ModInfo.author), "^1c^2h^3i^4c^5k^6e^7n / praet");
	snprintf(ModInfo.description, sizeof(ModInfo.description), "print top10 of given map/physic");
	snprintf(ModInfo.usage, sizeof(ModInfo.usage), "!top [map] [physic.mode] [startat##] [detail] [combined] [countries] [show]");
	AddCommand(&ModInfo, "!top", rs_cmd_top);
	return &ModInfo;
}

void print_user_cmd_to_chat(rs_client * cl, int to_all)
{
	if (to_all) {
		int i = 0;
		char buff[1024];
		buff[0] = '\0';

		while (cl->cmd_argv[i] != NULL) {
			sprintf(buff, "%s %s", buff, cl->cmd_argv[i]);
			i++;
		}

		if (strncmp(cl->cmd_argv[0], "login", 5) && strncmp(cl->cmd_argv[0], "!login", 6))
			SV_SendServerCommand(NULL, "chat \"%s^7:^2%s\n\"", cl->name, buff);
	}
}

void FreeParameterArray(char **temp)
{
	int i = 0;

	if (temp != NULL) {
		while (temp[i] != NULL) {
			free(temp[i]);
			i++;
		}
		free(temp);
	}
	temp = NULL;
}

char *va(char *format, ...)
{
	va_list argptr;
	static char string[2][32000];	// in case va is called by nested functions
	static int index = 0;
	char *buf;

	buf = string[index & 1];
	index++;

	va_start(argptr, format);
	vsnprintf(buf, sizeof(*string), format, argptr);
	va_end(argptr);

	return buf;
}

int atoi_dftime(char *dftime, int *i)
{
	int tmp = 0;

	while ((dftime[*i] != ':') && (dftime[*i] != '.') && (dftime[*i] != '\0')) {
		tmp *= 10;
		tmp += (dftime[*i] - '0');
		(*i)++;
	}
	return tmp;
}

int dftime_to_mstime(char *dftime)
{
	int a, b, c, i;
	int *p_i;
	p_i = &i;
	/*
	   read a number to the next ':' until a '\0' appears
	   we are expecting 0 to 2 ':' characters, 1 to 3 ints and one '\0' character.
	   atoi wants a '\0' terminated string. We don't have that.
	   Dare you adding whitspaces into the String.
	 */
	i = 0;

	a = atoi_dftime(dftime, p_i);
	if ((dftime[i] == ':') || (dftime[i] == '.'))	/* no need for going on. We are already at the end */
		i++;

	b = atoi_dftime(dftime, p_i);
	if ((dftime[i] == ':') || (dftime[i] == '.'))	/* no need for going on. We are already at the end */
		i++;

	if (dftime[i] == '\0') {
		a *= 1000;
		a += b;
		return a;
	}

	c = atoi_dftime(dftime, p_i);

	/*
	   converting to milliseconds and storing it in one integer.
	 */
	a *= 60000;
	a += (b * 1000);
	a += c;

	return a;
}

char *mstime_to_dftime(int mstime, int opt)
{
	int a, b, c, i, len;
	char *mem;

	a = mstime / 60000;
	b = (mstime % 60000) / 1000;
	c = ((mstime % 60000) % 1000);

	len = 0;
	i = a;
	while (i > 0) {
		i /= 10;
		len++;
	}

	if (a != 0)
		len++;		/* create space for a ':' */

	i = b;
	while (i > 0) {
		i /= 10;
		len++;
	}
	if (b == 0) {
		len++;		/* len would be 0, but we need at least 1 for a '0' */
		if (a != 0)
			len++;	/* there should be space for a leading zero if the time is longer than 1 minute */
	}
	len++;			/* create space for a ':' */
	len += 3;		/* the last 3 diges will always be there */

	mem = (char *)malloc(len + 1);
	if (opt == 1) {
		len++;
		if (a == 0)
			if (b == 0)
				if (c == 0)
					sprintf(mem, "-0:000");
				else
					sprintf(mem, "-0:%.3d", c);
			else
				sprintf(mem, "-%d:%.3d", b, c);
		else
			sprintf(mem, "-%d:%.2d:%.3d", a, b, c);	/* there should be a leading zero at the 'seconds'-part wenn the time is longer than 1 minute */
	}
	if (opt == 2) {
		len++;
		if (a == 0)
			if (b == 0)
				if (c == 0)
					sprintf(mem, "+0:000");
				else
					sprintf(mem, "+0:%.3d", c);
			else
				sprintf(mem, "+%d:%.3d", b, c);
		else
			sprintf(mem, "+%d:%.2d:%.3d", a, b, c);	/* there should be a leading zero at the 'seconds'-part wenn the time is longer than 1 minute */
	}
	if (opt == 0) {
		if (a == 0)
			if (b == 0)
				if (c == 0)
					sprintf(mem, "0:000");
				else
					sprintf(mem, "0:%.3d", c);
			else
				sprintf(mem, "%d:%.3d", b, c);
		else
			sprintf(mem, "%d:%.2d:%.3d", a, b, c);	/* there should be a leading zero at the 'seconds'-part wenn the time is longer than 1 minute */
	}
	return mem;
}

int GetELOEnabled( void ) {

	int result = 0;
	MYSQL sqlcon;
	MYSQL_RES *results;
	MYSQL_ROW fetch;

	if (sql_connect(&sqlcon)) {
		results = sql_get_query(&sqlcon, "SELECT id \
        FROM q3_users \
        WHERE id='297' AND hardware LIKE '%%enabled%%'");
		if (results) {
			fetch = mysql_fetch_row(results);
			if (fetch) {
				//ELO is enabled
				result = 1;
			}
			sql_free_result(results);
		}
	}

	sql_close(&sqlcon);

	return result;
}

/*
==================
str_clean
remove " \n and clear qcolors ^1
==================
char * test = "^11^22^33^44^55^66^77^88^99^00test test ^7";
test = (char *)str_clean(test);
Com_Printf( "test:%s\n", test );
Com_Printf( "str_clean(test):%s\n", str_clean(test) );
*/
char * str_clean(char * str)
{
	char *string;
	char *result;
//	char * s1;
//	char * s2;
//	char * search;
    if (strlen(str)==0) return str;

    // append a terminator '\0' to input string
    string = (char *) malloc (strlen(str)+1);
	strcpy(string,str);

    // check if its corrupt
    int type = 1;

	int i = 0;
	int length = strlen(string);
	do
	{
		type = isascii(string[i]);
//        printf ("%c=", string[i]);
//        printf ("%o ", string[i]);
		i++;
	} while ((i < length) && (type == 1));
    if (type == 0)
        return "Detected corruption";

	char * pos = string;

    i = 1;
	while (*pos != 0)
	{
		if (*pos=='^')
		{
//TODO:			if( pos+1 <= 9 ) //is this a color code? handle ^^1
   			*pos='"';
   			pos++;
			i++;
   			*pos='"';
		}
		if (*pos<32 && *pos>13 )
		{
    		*pos='"';
		}
		if (*pos=='\n')
		{
		    // only trim trailing carriage return
		    if (i == strlen(str))
    			*pos='"';
		}
		pos++;
		i++;
	}


    if (!strstr(string, "\"")){
        return string;
    }


    // strip " from message
	char * cp = string;
    result = (char *)malloc (strlen(string)+1);
    memset(result, '\0', sizeof(result));

	while (*cp!=0)
	{
	    if (*cp != '"')
	    {
	        // copy the char to result
		    strncat(result, cp, 1);
        }
        cp++;

    }
//  strncpy(str,result, sizeof(str));
//	return str;
	strcpy(string,result);
	return string;
}

#ifdef WIN32
}
#endif

