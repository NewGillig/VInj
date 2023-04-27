int16_t *ff_h263_pred_motion(MpegEncContext *s, int block, int dir, int *px, int *py)
{
    int wrap;
    int16_t *A, *B, *C, (*mot_val)[2];
    static const int off[4] = {2, 1, 1, -1};
    wrap = s->b8_stride;
    mot_val = s->current_picture.f.motion_val[dir] + s->block_index[block];
    A = mot_val[-1];
    if (s->first_slice_line && block < 3)
    {
        if (block == 0)
        {
            if (s->mb_x == s->resync_mb_x)
            {
                *px = *py = 0;
            }
            else if (s->mb_x + 1 == s->resync_mb_x && s->h263_pred)
            {
                C = mot_val[off[block] - wrap];
                if (s->mb_x == 0)
                {
                    *px = C[0];
                    *py = C[1];
                }
                else
                {
                    *px = mid_pred(A[0], 0, C[0]);
                    *py = mid_pred(A[1], 0, C[1]);
                }
            }
            else
            {
                *px = A[0];
                *py = A[1];
            }
        }
        else if (block == 1)
        {
            if (s->mb_x + 1 == s->resync_mb_x && s->h263_pred)
            {
                C = mot_val[off[block] - wrap];
                *px = mid_pred(A[0], 0, C[0]);
                *py = mid_pred(A[1], 0, C[1]);
            }
            else
            {
                *px = A[0];
                *py = A[1];
            }
        }
        else
        {
            B = mot_val[-wrap];
            C = mot_val[off[block] - wrap];
            if (s->mb_x == s->resync_mb_x)
                A[0] = A[1] = 0;
            *px = mid_pred(A[0], B[0], C[0]);
            *py = mid_pred(A[1], B[1], C[1]);
        }
    }
    else
    {
        B = mot_val[-wrap];
        C = mot_val[off[block] - wrap];
        *px = mid_pred(A[0], B[0], C[0]);
        *py = mid_pred(A[1], B[1], C[1]);
    }
    return *mot_val;
}