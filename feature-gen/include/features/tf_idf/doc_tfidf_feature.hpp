#pragma once
/**
 * tfidf
 */
class doc_tfidf_feature : public doc_feature {
  double _calculate_tfidf(double d_f, double t_idf, double dlen) {
    double doc_norm = 1.0 / dlen;
    double w_dq = 1.0 + std::log(d_f);
    double w_Qq = std::log(1.0 + ((double)_num_docs / t_idf));

    return (doc_norm * w_dq * w_Qq);
  }

public:
  doc_tfidf_feature(indri_index &idx) : doc_feature(idx) {}

  void compute(doc_entry &doc, FreqsEntry &freqs) {

    _score_reset();

    for (auto &q : freqs.q_ft) {
      // skip non-existent terms
      if (q.first == 0) {
        continue;
      }

      if (freqs.d_ft.find(q.first) == freqs.d_ft.end()) {
        continue;
      }

      _score_doc += _calculate_tfidf(freqs.d_ft.at(q.first),
                                     index.termCount(index.term(q.first)),
                                     freqs.doc_length);

      // Score document title, heading, inlink fields
      for (const std::string &field_str : _fields) {
        int field_id = index.field(field_str);
        if (field_id < 1) {
          // field is not indexed
          continue;
        }

        if (0 == freqs.field_len[field_id]) {
          continue;
        }
        if (freqs.f_ft.find(std::make_pair(field_id, q.first)) == freqs.f_ft.end()) {
          continue;
        }

        int field_term_cnt =
            index.fieldTermCount(field_str, index.term(q.first));
        if (0 == field_term_cnt) {
          continue;
        }

        double field_score =
            _calculate_tfidf(freqs.f_ft.at(std::make_pair(field_id, q.first)), field_term_cnt, freqs.field_len[field_id]);
        _accumulate_score(field_str, field_score);
      }
    }

    doc.tfidf = _score_doc;
    doc.tfidf_body = _score_body;
    doc.tfidf_title = _score_title;
    doc.tfidf_heading = _score_heading;
    doc.tfidf_inlink = _score_inlink;
    doc.tfidf_a = _score_a;
  }
};