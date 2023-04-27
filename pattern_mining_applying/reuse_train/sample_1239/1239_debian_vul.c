static int remoteStreamHandleWriteData(struct qemud_client *client, struct qemud_client_stream *stream, struct qemud_client_message *msg)
{
    remote_error rerr;
    int ret;
    VIR_DEBUG("stream=%p proc=%d serial=%d len=%d offset=%d", stream, msg->hdr.proc, msg->hdr.serial, msg->bufferLength, msg->bufferOffset);
    memset(&rerr, 0, sizeof rerr);
    ret = virStreamSend(stream->st, msg->buffer + msg->bufferOffset, msg->bufferLength - msg->bufferOffset);
    if (ret > 0)
    {
        msg->bufferOffset += ret;
        if (msg->bufferOffset < msg->bufferLength)
            return 1;
    }
    else if (ret == -2)
    {
        return 1;
    }
    else
    {
        VIR_INFO0("Stream send failed");
        stream->closed = 1;
        remoteDispatchConnError(&rerr, client->conn);
        return remoteSerializeReplyError(client, &rerr, &msg->hdr);
    }
    return 0;
}