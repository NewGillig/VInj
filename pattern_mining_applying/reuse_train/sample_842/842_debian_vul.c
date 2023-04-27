static uint dump_routines_for_db(char *db)
{
    char query_buff[QUERY_LENGTH];
    const char *routine_type[] = {"FUNCTION", "PROCEDURE"};
    char db_name_buff[NAME_LEN * 2 + 3], name_buff[NAME_LEN * 2 + 3];
    char *routine_name;
    int i;
    FILE *sql_file = md_result_file;
    MYSQL_RES *routine_res, *routine_list_res;
    MYSQL_ROW row, routine_list_row;
    char db_cl_name[MY_CS_NAME_SIZE];
    int db_cl_altered = FALSE;
    DBUG_ENTER("dump_routines_for_db");
    DBUG_PRINT("enter", ("db: '%s'", db));
    mysql_real_escape_string(mysql, db_name_buff, db, (ulong)strlen(db));
    print_comment(sql_file, 0, "\n--\n-- Dumping routines for database '%s'\n--\n", db);
    if (lock_tables)
        mysql_query(mysql, "LOCK TABLES mysql.proc READ");
    if (fetch_db_collation(db, db_cl_name, sizeof(db_cl_name)))
        DBUG_RETURN(1);
    if (switch_character_set_results(mysql, "binary"))
        DBUG_RETURN(1);
    if (opt_xml)
        fputs("\t<routines>\n", sql_file);
    for (i = 0; i <= 1; i++)
    {
        my_snprintf(query_buff, sizeof(query_buff), "SHOW %s STATUS WHERE Db = '%s'", routine_type[i], db_name_buff);
        if (mysql_query_with_error_report(mysql, &routine_list_res, query_buff))
            DBUG_RETURN(1);
        if (mysql_num_rows(routine_list_res))
        {
            while ((routine_list_row = mysql_fetch_row(routine_list_res)))
            {
                routine_name = quote_name(routine_list_row[1], name_buff, 0);
                DBUG_PRINT("info", ("retrieving CREATE %s for %s", routine_type[i], name_buff));
                my_snprintf(query_buff, sizeof(query_buff), "SHOW CREATE %s %s", routine_type[i], routine_name);
                if (mysql_query_with_error_report(mysql, &routine_res, query_buff))
                    DBUG_RETURN(1);
                while ((row = mysql_fetch_row(routine_res)))
                {
                    DBUG_PRINT("info", ("length of body for %s row[2] '%s' is %zu", routine_name, row[2] ? row[2] : "(null)", row[2] ? strlen(row[2]) : 0));
                    if (row[2] == NULL)
                    {
                        print_comment(sql_file, 1, "\n-- insufficient privileges to %s\n", query_buff);
                        print_comment(sql_file, 1, "-- does %s have permissions on mysql.proc?\n\n", current_user);
                        maybe_die(EX_MYSQLERR, "%s has insufficent privileges to %s!", current_user, query_buff);
                    }
                    else if (strlen(row[2]))
                    {
                        if (opt_xml)
                        {
                            if (i)
                                print_xml_row(sql_file, "routine", routine_res, &row, "Create Procedure");
                            else
                                print_xml_row(sql_file, "routine", routine_res, &row, "Create Function");
                            continue;
                        }
                        if (opt_drop)
                            fprintf(sql_file, "; \ n ", routine_type[i], routine_name);
                        if (mysql_num_fields(routine_res) >= 6)
                        {
                            if (switch_db_collation(sql_file, db, "; ", db_cl_name, row[5], &db_cl_altered))
                            {
                                DBUG_RETURN(1);
                            }
                            switch_cs_variables(sql_file, "; ", row[3], row[3], row[4]);
                        }
                        else
                        {
                            fprintf(sql_file, "--\n"
                                              "-- WARNING: old server version. "
                                              "The following dump may be incomplete.\n"
                                              "--\n");
                        }
                        switch_sql_mode(sql_file, "; ", row[1]);
                        fprintf(sql_file, "DELIMITER ; ; \ n "
                                          " % s ; ; \ n "
                                          " DELIMITER ; \ n ",
                                (const char *)row[2]);
                        restore_sql_mode(sql_file, "; ");
                        if (mysql_num_fields(routine_res) >= 6)
                        {
                            restore_cs_variables(sql_file, "; ");
                            if (db_cl_altered)
                            {
                                if (restore_db_collation(sql_file, db, "; ", db_cl_name))
                                    DBUG_RETURN(1);
                            }
                        }
                    }
                }
                mysql_free_result(routine_res);
            }
        }
        mysql_free_result(routine_list_res);
    }
    if (opt_xml)
    {
        fputs("\t</routines>\n", sql_file);
        check_io(sql_file);
    }
    if (switch_character_set_results(mysql, default_charset))
        DBUG_RETURN(1);
    if (lock_tables)
        (void)mysql_query_with_error_report(mysql, 0, "UNLOCK TABLES");
    DBUG_RETURN(0);
}