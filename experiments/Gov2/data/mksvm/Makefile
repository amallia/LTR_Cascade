# config.sh
#
CORPUS_PATH := /research/remote/collections/TREC/gov2/gov2-corpus

# common.sh
#
BIN := ../../../../external/local/bin
CONFIGDIR := ../config

# REPO_PATH := gov2_indri
# DUMP_PATH := gov2_dump
# LINKS_PATH := gov2_links
# WAND_PATH := gov2_wand

# BIGRAM_POSTINGS := gov2_bigram_u8.inv
# UNIGRAM_FILE := gov2_unigram.txt
# BIGRAM_FILE := gov2_bigram.txt
# TERMFEAT_FILE := gov2_termfeat.csv
# DOCFEAT_FILE := gov2_docfeat.csv
# QRY := gov2-all-kstem.qry
# STAGE0_RUN := stage0.run
# PAGERANK_PRIOR := pagerank.prior

.PHONY: all links indri prior dump unigram bigram wand termfeat docfeat clean

.DELETE_ON_ERROR:

all: all.svm

links: gov2_links/.done
indri: gov2_indri/manifest
prior: gov2_indri/prior/pagerank
dump: gov2_dump/.done
unigram: gov2_unigram.txt
bigram: gov2_bigram.txt
wand: gov2_wand/.done
termfeat: gov2_termfeat.csv
docfeat: gov2_docfeat.csv


gov2_links/.done:
	$(BIN)/harvestlinks -corpus=$(CORPUS_PATH) -output=gov2_links
	touch $@

gov2_indri/manifest: gov2_links/.done
	rm -rf gov2_indri
	$(BIN)/IndriBuildIndex $(CONFIGDIR)/gov2.param

gov2_indri/prior/pagerank: gov2_indri/manifest pagerank.prior
	$(BIN)/makeprior -index=gov2_indri -input=pagerank.prior -name=pagerank

pagerank.prior: pagerank.prior.xz
	xz -fdk $<

gov2_dump/.done: gov2_indri/prior/pagerank
	$(BIN)/dump_indri gov2_indri gov2_dump
	touch $@

# Unigram features
gov2_unigram.txt: gov2_dump/.done
	$(BIN)/fgen_term -i gov2_dump/text.inv -d gov2_dump/doc_lens.txt \
	    -c '$(shell awk "{print $$2}" gov2_dump/global.txt)' > $@

# Dump bigram unordered window of 8 posting list for `fgen_bigram`
gov2_bigram_u8.inv: gov2-all-kstem.qry
	awk -F\; '{gsub(" ", ","); print $$2}' $< > qry_tmp
	$(BIN)/dump_bigram -i gov2_indri -q qry_tmp -w8 -u -s _tmp
	rm qry_tmp
	mv u8_tmp.txt $@

gov2_bigram.txt: gov2_bigram_u8.inv
	$(BIN)/fgen_bigram -i $< -d gov2_dump/doc_lens.txt \
	    -c '$(shell awk "{print $$2}" gov2_dump/global.txt)' | sed 's/-nan/0.00000/g' > $@

# Perform a Stage0 run and replace 'Q0' with relevance labels
gov2_wand/.done: gov2_indri/manifest
	$(BIN)/mk_wand_idx gov2_indri gov2_wand
	touch $@

stage0.run: gov2_wand/.done gov2-all-kstem.qry
	$(BIN)/wand_search -c gov2_wand -q gov2-all-kstem.qry -k 10000 -i
	rm wand-time.log
	./label.awk gov2.qrels wand-trec.run > $@

# Unigram and bigram features
gov2_termfeat.csv: gov2_indri/manifest gov2_unigram.txt gov2_bigram.txt
	$(BIN)/preret_csv gov2-all-kstem.qry gov2_unigram.txt gov2_bigram.txt gov2_indri \
	    | sed -E 's/\-?nan/0.00000/g' > $@

# Document features
LINES := 372600

split/.done: stage0.run
	mkdir -p split
	(cd split; split -dl $(LINES) ../stage0.run)
	touch $@

gov2_docfeat.csv: gov2_indri/manifest stage0.run split/.done
	for f in split/x??; do $(BIN)/fgtrain gov2-all-kstem.qry $$f gov2_indri; done > $@

# Combine into SVM light format
termfeat.tmp: gov2_docfeat.csv gov2_termfeat.csv
	./termfeat_expand.awk gov2_docfeat.csv gov2_termfeat.csv | sed -E 's/[^,]+,//' > $@

all.csv: gov2_docfeat.csv termfeat.tmp
	paste -d, gov2_docfeat.csv termfeat.tmp > $@

all.svm: all.csv
	./csv2svm.awk $< > $@

# Clean up
clean:
	rm -rf gov2_dump gov2_indri gov2_links gov2_wand pagerank.ivl *.csv *.svm 
