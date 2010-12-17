matrix = function(path) {
  require(Matrix)
  f = file(path)
  data = scan(f, what="char", sep=" ", quiet=TRUE, comment.char="", skip=3)
  close(f)
  rawngrams = data[c(TRUE, FALSE)]
  origin = data[c(FALSE, TRUE)]
  processNgram = function(cv) {
    ret = cv[3]
    names(ret) = cv[2]
    return(ret)
  }
  ngrams = lapply(strsplit(rawngrams, ",", fixed=TRUE), function(obj) sapply(strsplit(obj, ":", fixed=TRUE), processNgram))
  allNgrams = unique(unlist(lapply(ngrams, function(ngram) names(ngram)), use.names=FALSE))
  indices = unlist(lapply(ngrams, function(ngram) match(names(ngram), allNgrams)), use.names=FALSE)
  # generate a matrix in ml-style: rows are the features, cols are the samples
  mat = sparseMatrix(indices, rep(1:length(ngrams), sapply(ngrams, length)), x= as.numeric(unlist(ngrams, use.names=FALSE)), dims=c(length(allNgrams), length(ngrams)), dimnames=list(allNgrams, origin))
  return(mat)
}
