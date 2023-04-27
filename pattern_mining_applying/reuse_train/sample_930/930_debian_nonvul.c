static void add_range(fz_context *ctx, pdf_cmap *cmap, unsigned int low, unsigned int high, unsigned int out, int check_for_overlap, int many)
{
    int current;
    cmap_splay *tree;
    if (low > high)
    {
        fz_warn(ctx, "range limits out of range in cmap %s", cmap->cmap_name);
        return;
    }
    tree = cmap->tree;
    if (cmap->tlen)
    {
        unsigned int move = cmap->ttop;
        unsigned int gt = EMPTY;
        unsigned int lt = EMPTY;
        if (check_for_overlap)
        {
            do
            {
                current = move;
                if (low <= tree[current].low && tree[current].low <= high)
                {
                    tree[current].out += high + 1 - tree[current].low;
                    tree[current].low = high + 1;
                    if (tree[current].low > tree[current].high)
                    {
                        move = delete_node(cmap, current);
                        current = EMPTY;
                        continue;
                    }
                }
                else if (low <= tree[current].high && tree[current].high <= high)
                {
                    tree[current].high = low - 1;
                    assert(tree[current].low <= tree[current].high);
                }
                else if (tree[current].low < low && high < tree[current].high)
                {
                    int new_high = tree[current].high;
                    tree[current].high = low - 1;
                    add_range(ctx, cmap, high + 1, new_high, tree[current].out + high + 1 - tree[current].low, 0, tree[current].many);
                }
                if (tree[current].low > high)
                {
                    move = tree[current].left;
                    gt = current;
                }
                else
                {
                    move = tree[current].right;
                    lt = current;
                }
            } while (move != EMPTY);
        }
        else
        {
            do
            {
                current = move;
                if (tree[current].low > high)
                {
                    move = tree[current].left;
                    gt = current;
                }
                else
                {
                    move = tree[current].right;
                    lt = current;
                }
            } while (move != EMPTY);
        }
        if (!many)
        {
            if (lt != EMPTY && !tree[lt].many && tree[lt].high == low - 1 && tree[lt].out - tree[lt].low == out - low)
            {
                tree[lt].high = high;
                if (gt != EMPTY && !tree[gt].many && tree[gt].low == high + 1 && tree[gt].out - tree[gt].low == out - low)
                {
                    tree[lt].high = tree[gt].high;
                    delete_node(cmap, gt);
                }
                goto exit;
            }
            if (gt != EMPTY && !tree[gt].many && tree[gt].low == high + 1 && tree[gt].out - tree[gt].low == out - low)
            {
                tree[gt].low = low;
                tree[gt].out = out;
                goto exit;
            }
        }
    }
    else
        current = EMPTY;
    if (cmap->tlen == cmap->tcap)
    {
        int new_cap = cmap->tcap ? cmap->tcap * 2 : 256;
        tree = cmap->tree = fz_resize_array(ctx, cmap->tree, new_cap, sizeof *cmap->tree);
        cmap->tcap = new_cap;
    }
    tree[cmap->tlen].low = low;
    tree[cmap->tlen].high = high;
    tree[cmap->tlen].out = out;
    tree[cmap->tlen].parent = current;
    tree[cmap->tlen].left = EMPTY;
    tree[cmap->tlen].right = EMPTY;
    tree[cmap->tlen].many = many;
    cmap->tlen++;
    if (current == EMPTY)
        cmap->ttop = 0;
    else if (tree[current].low > high)
        tree[current].left = cmap->tlen - 1;
    else
    {
        assert(tree[current].high < low);
        tree[current].right = cmap->tlen - 1;
    }
    move_to_root(tree, cmap->tlen - 1);
    cmap->ttop = cmap->tlen - 1;
exit:
{
}
}