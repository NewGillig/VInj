static my_bool get_view_structure(char *table, char *db)
{
    MYSQL_RES *table_res;
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    char *result_table, *opt_quoted_table;
    char table_buff[NAME_LEN * 2 + 3];
    char table_buff2[NAME_LEN * 2 + 3];
    char query[QUERY_LENGTH];
    FILE *sql_file = md_result_file;
    DBUG_ENTER("get_view_structure");
    if (opt_no_create_info)
        DBUG_RETURN(0);
    verbose_msg("-- Retrieving view structure for table %s...\n", table);
    dynstr_append_checked(&insert_pat, (opt_quoted || opt_keywords) ? "1" : "0");
    opt_quoted_table = quote_name(table, table_buff2, 0);
    if (switch_character_set_results(mysql, "binary"))
        DBUG_RETURN(1);
    my_snprintf(query, sizeof(query), "SHOW CREATE TABLE %s", result_table);
    if (mysql_query_with_error_report(mysql, &table_res, query))
    {
        switch_character_set_results(mysql, default_charset);
        DBUG_RETURN(0);
    }
    field = mysql_fetch_field_direct(table_res, 0);
    if (strcmp(field->name, "View") != 0)
    {
        mysql_free_result(table_res);
        switch_character_set_results(mysql, default_charset);
        verbose_msg("-- It's base table, skipped\n");
        DBUG_RETURN(0);
    }
    if (path)
    {
        if (!(sql_file = open_sql_file_for_table(table, O_WRONLY)))
        {
            mysql_free_result(table_res);
            DBUG_RETURN(1);
        }
        write_header(sql_file, db);
    }
    print_comment(sql_file, 0, "\n--\n-- Final view structure for view %s\n--\n\n", result_table);
    fprintf(sql_file, "; \ n ", opt_quoted_table);
    if (opt_drop)
    {
        fprintf(sql_file, "; \ n ", opt_quoted_table);
        check_io(sql_file);
    }
    my_snprintf(query, sizeof(query), "SELECT CHECK_OPTION, DEFINER, SECURITY_TYPE, "
                                      " CHARACTER_SET_CLIENT, COLLATION_CONNECTION "
                                      "FROM information_schema.views "
                                      "WHERE table_name=\"%s\" AND table_schema=\"%s\"",
                table, db);
    if (mysql_query(mysql, query))
    {
        row = mysql_fetch_row(table_res);
        fprintf(sql_file, "; \ n ", row[1]);
        check_io(sql_file);
        mysql_free_result(table_res);
    }
    else
    {
        char *ptr;
        ulong *lengths;
        char search_buf[256], replace_buf[256];
        ulong search_len, replace_len;
        DYNAMIC_STRING ds_view;
        row = mysql_fetch_row(table_res);
        lengths = mysql_fetch_lengths(table_res);
        init_dynamic_string_checked(&ds_view, row[1], lengths[1] + 1, 1024);
        mysql_free_result(table_res);
        if (!(table_res = mysql_store_result(mysql)) || !(row = mysql_fetch_row(table_res)))
        {
            if (table_res)
                mysql_free_result(table_res);
            dynstr_free(&ds_view);
            DB_error(mysql, "when trying to save the result of SHOW CREATE TABLE in ds_view.");
            DBUG_RETURN(1);
        }
        lengths = mysql_fetch_lengths(table_res);
        if (strcmp(row[0], "NONE"))
        {
            ptr = search_buf;
            search_len = (ulong)(strxmov(ptr, "WITH ", row[0], " CHECK OPTION", NullS) - ptr);
            ptr = replace_buf; replace_len = ( ulong ) ( strxmov ( ptr , "*/\n\n\n; \ n " " ; \ n " " ; \ n " " ; \ n " " ; \ n " " ; \ n " " ; \ n " " ; \ n " " ; \ n " " ; \ n " , ( const char * ) row [ 3 ] , ( const char * ) row [ 3 ] , ( const char * ) row [ 4 ] , ( const char * ) ds_view . str ) ; check_io ( sql_file ) ; mysql_free_result ( table_res ) ; dynstr_free ( & ds_view ) ;
        }
        if (switch_character_set_results(mysql, default_charset))
            DBUG_RETURN(1);
        if (sql_file != md_result_file)
        {
            fputs("\n", sql_file);
            write_footer(sql_file);
            my_fclose(sql_file, MYF(MY_WME));
        }
        DBUG_RETURN(0);