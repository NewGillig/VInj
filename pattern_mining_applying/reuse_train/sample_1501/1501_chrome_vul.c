int vp9_rc_regulate_q(const VP9_COMP *cpi, int target_bits_per_frame, int active_best_quality, int active_worst_quality)
{
    const VP9_COMMON *const cm = &cpi->common;
    int q = active_worst_quality;
    int last_error = INT_MAX;
    int i, target_bits_per_mb;
    const double correction_factor = get_rate_correction_factor(cpi);
    target_bits_per_mb = ((uint64_t)target_bits_per_frame << BPER_MB_NORMBITS) / cm->MBs;
    i = active_best_quality;
    do
    {
        const int bits_per_mb_at_this_q = (int)vp9_rc_bits_per_mb(cm->frame_type, i, correction_factor);
        if (bits_per_mb_at_this_q <= target_bits_per_mb)
        {
            if ((target_bits_per_mb - bits_per_mb_at_this_q) <= last_error)
                q = i;
            else
                q = i - 1;
            break;
        }
        else
        {
            last_error = bits_per_mb_at_this_q - target_bits_per_mb;
        }
    } while (++i <= active_worst_quality);
    return q;
}