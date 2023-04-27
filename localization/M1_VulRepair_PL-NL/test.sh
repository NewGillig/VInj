python vulrepair_main.py \
    --output_dir=./saved_models \
    --model_name=model.bin \
    --tokenizer_name=Salesforce/codet5-base \
    --model_name_or_path=Salesforce/codet5-base \
    --do_test \
    --encoder_block_size 512 \
    --decoder_block_size 512 \
    --num_beams=1 \
    --eval_batch_size 1
