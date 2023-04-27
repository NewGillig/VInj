static int remoteStreamHandleFinish(struct qemud_client *client, struct qemud_client_stream *stream, struct qemud_client_message *msg)
{
    remote_error rerr;
    int ret;
    VIR_DEBUG("stream=%p proc=%d serial=%d", stream, msg->hdr.proc, msg->hdr.serial);
    memset(&rerr, 0, sizeof rerr);
    stream->closed = 1;
    virStreamEventRemoveCallback(stream->st);
    ret = virStreamFinish(stream->st);
    if (ret < 0)
    {
        remoteDispatchConnError(&rerr, client->conn);
        return remoteSerializeReplyError(client, &rerr, &msg->hdr);
    }
    else
    {
        if (remoteSendStreamData(client, stream, NULL, 0) < 0)
            return -1;
    }
    return 0;
}