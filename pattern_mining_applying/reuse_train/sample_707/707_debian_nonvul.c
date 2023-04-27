static int token_continue(i_ctx_t *i_ctx_p, scanner_state *pstate, bool save)
{
    os_ptr op = osp;
    int code;
    ref token;
    make_null(osp);
    pop(1);
again:
    code = gs_scan_token(i_ctx_p, &token, pstate);
    op = osp;
    switch (code)
    {
    default:
        if (code > 0)
            code = gs_note_error(gs_error_syntaxerror);
        gs_scanner_error_object(i_ctx_p, pstate, &i_ctx_p->error_object);
        break;
    case scan_BOS:
        code = 0;
    case 0:
        push(2);
        ref_assign(op - 1, &token);
        make_true(op);
        break;
    case scan_EOF:
        push(1);
        make_false(op);
        code = 0;
        break;
    case scan_Refill:
        code = gs_scan_handle_refill(i_ctx_p, pstate, save, ztoken_continue);
        switch (code)
        {
        case 0:
            goto again;
        case o_push_estack:
            return code;
        }
        break;
    }
    if (code <= 0 && !save)
    {
        ifree_object(pstate, "token_continue");
    }
    return code;
}