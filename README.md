# Audiq script
Usage: ./audiq [options] folder_with_samples [output_file] [weight_1 weight_2 weight_3]  

## Options:  
  - -h, --help Show this message.  
  - -o, --one  Use one_dataset mode (recommendation based on sample type {melody, percussion or vocal} ).  
  - -p, --print  Print result to stdout.  
  - -n, --no-processing Don't extract descriptors and datasets creating (use this option if you already have datasets and want
  to test audiq with different modes or weights).  

## Notes:  
   - If 'output_file' arg was not parsed then the result file name will be result_w1_w2_w3_[one|many]_ds.yaml.  
   - Weights meaning: weight_1, weight_2, weight_3 (weight coefficients in metric used by audiq) corresponds to low-level signal
   descriptors (such energy...), timbre and highlevel descriptors (e.g. sample tye, bass).  
  
## Audiq dataset mode:  
   - One dataset means that all samples will be gathed in one dataset.  
   - Many dataset means that similar samples must belong to one class {melody, percussion, vocal}.  
   - Many dataset mode can do better recommendation but sometimes analyser make wrong classification and sample of
   "real" type percussion become vocal, in this case recommendation is strange, but if classification is right then analyser
   use some specific classification models for each class and can do good recommendation.  
   - One dataset mode is more stable in comparison with many dataset mode.  
   - Default values for weights 1, 2, 3. Default dataset mode - many datasets.  

## Recommendation:  
   - Use audiq with default settings: ./audiq folder_with_samples.  
   - After this you will have datasets, so you can vary params, for example:
    "./audiq -no folder_with_sample 1 1 1" - one dataset mode with weights 1 1 1.
  
