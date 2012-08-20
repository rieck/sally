% Main function 
function x = octave(ifile, ofile, nlen, delim)
    in = fopen(ifile);
    out = fopen(ofile, 'w');

    while ~feof(in)
        line = fgetl(in);
        if length(delim) == 0 
            n = extract_ngrams(line, nlen);
        else
            n = extract_wgrams(line, nlen, delim);
        end
        print_libsvm(out, n, 1.0);
    end

    fclose(in);
    fclose(out);
end

% Extract n-grams of characters
function ngrams = extract_ngrams(str, nlen)
    ngrams = zeros(2^16, 1); % Sparse matrices are slower :(
    bases = uint16(mod(33.^(1:nlen), 2^16));

    for i = 1:length(str) - nlen + 1
        ngram = uint16(str(i:i + nlen - 1));
	% Simple hash function for strings 
        hash = mod(sum(ngram .* bases), 2^16) + 1;
	ngrams(hash) = ngrams(hash) + 1.0;
    end
end

% Extract n-grams of words
function ngrams = extract_wgrams(str, nlen, delim)
    ngrams = zeros(2^16, 1); % Sparse matrices are slower :(
    bases = uint16(33.^(1:10000));

    % Simple hack instead of full decoding
    delim = strrep(delim, '%0a', ' ');
    delim = strrep(delim, '%0d', ' ');
    for d = delim
       str = strrep(str, d, ' ');
    end

    list = strfind(str, ' ');
    for i = 1:length(list) - nlen
	ngram = uint16(str(list(i):list(i + nlen)));
	% Simple hash function for strings 
        hash = mod(sum(ngram .* bases(1:length(ngram))), 2^16) + 1;
	ngrams(hash) = ngrams(hash) + 1.0;
    end
end

% Output vectors in libsvm format
function print_libsvm(out, ngrams, label)
    fprintf(out, '%d ', label);
    x = find(ngrams > 0);
    y = ngrams(x);
    fprintf(out, '%d:%g ', [x';y']);
    fprintf(out, '\n');
end
