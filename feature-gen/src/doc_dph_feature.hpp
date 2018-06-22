#ifndef DOC_DPH_FEATURE_HPP
#define DOC_DPH_FEATURE_HPP

#include "doc_feature.hpp"

/**
 * DFR: DPH
 */
class doc_dph_feature : public doc_feature {
  double _calculate_dph(uint32_t d_f, uint64_t c_f, uint32_t dlen) {
    double l, r, prime, rsv;

    l = std::log(1.0 + (double)c_f / _num_docs);
    r = std::log(1.0 + (double)_num_docs / (double)c_f);
    prime = d_f * std::log(1.0 + _avg_doc_len / (double)dlen);
    rsv = (l + prime * r) / (prime + 1.0);

    return rsv;
  }

public:
  doc_dph_feature(indri_index &idx) : doc_feature(idx) {}

  void compute(fat_cache_entry &doc, freqs_entry &freqs) {

    _score_reset();


    const indri::index::TermList *term_list = doc.term_list;
    auto &doc_terms = term_list->terms();

    for (auto &q : freqs.q_ft) {
      // skip non-existent terms
      if (q.first == 0) {
        continue;
      }

      if (0 == freqs.d_ft.at(q.first)) {
        continue;
      }

      _score_doc +=
          _calculate_dph(freqs.d_ft.at(q.first), index.termCount(index.term(q.first)),
                         doc_terms.size());

      // Score document fields
      auto fields = term_list->fields();
      for (const std::string &field_str : _fields) {
        int field_id = index.field(field_str);
        size_t field_len = 0;
        if (field_id < 1) {
          // field is not indexed
          continue;
        }
        for (auto &f : fields) {
          if (f.id != static_cast<size_t>(field_id)) {
            continue;
          }

          field_len += f.end - f.begin;
        }

        if (0 == field_len) {
          continue;
        }
        if (0 == freqs.f_ft.at(q.first)) {
          continue;
        }

        int field_term_cnt =
            index.fieldTermCount(field_str, index.term(q.first));
        if (0 == field_term_cnt) {
          continue;
        }

        double field_score =
            _calculate_dph(freqs.f_ft.at(q.first), field_term_cnt, field_len);
        _accumulate_score(field_str, field_score);
      }
    }

    doc.dph = _score_doc;
    doc.dph_body = _score_body;
    doc.dph_title = _score_title;
    doc.dph_heading = _score_heading;
    doc.dph_inlink = _score_inlink;
    doc.dph_a = _score_a;
  }
};

#endif
