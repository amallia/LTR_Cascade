#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>


#include "indri/Index.hpp"

class document_features {
  using indri_index = indri::index::Index;

  indri_index &index;

  // The frequency of query terms within the <title> tag
  size_t tag_title_qry_count = 0;
  // The frequency of query terms within the <mainbody> tag
  size_t tag_mainbody_qry_count = 0;
  // The frequency of query terms within the h1-h4 tags (called heading by
  // Indri)
  size_t tag_heading_qry_count = 0;
  // The frequency of query terms within the inlinks
  size_t tag_inlink_qry_count = 0;


  enum field_id : uint16_t {
    F_STR_NONE = 0,
    F_STR_TITLE,
    F_STR_HEADING,
    F_STR_MAINBODY,
    F_STR_INLINK,

    MARKER_STOP_QRY_COUNT,

    F_STR_APPLET,
    F_STR_OBJECT,
    F_STR_EMBED,
  };
  static std::map<std::string, uint16_t> field_lookup;

public:
  document_features(indri_index &i) : index(i) {}

  void compute(doc_entry &doc, FreqsEntry &freqs) {

    /*
     * List of fields for the current document. The field `id` indicates which
     * field it is, e.g. title, heading, etc
     */

    const std::vector<std::string> idx_fields = {
        "title", "heading", "mainbody", "inlink", "applet", "object", "embed"};
    for (auto field_str : idx_fields) {
      int field_id = index.field(field_str);
      if (field_id < 1) {
        // field does not exist
        std::cerr << "field '" << field_str << "' does not exist" << std::endl;
        continue;
      }

      auto i = field_lookup.at(field_str);
      if (i > MARKER_STOP_QRY_COUNT) {
        // only count the number of tags
        continue;
      }

      size_t qry_term_count = 0;
      for (auto &q : freqs.q_ft) {
        qry_term_count += freqs.f_ft[{field_id, q.first}];
      }

      set_tag_qry_count(field_str, qry_term_count);
    }

    doc.tag_title_qry_count = tag_title_qry_count;
    doc.tag_heading_qry_count = tag_heading_qry_count;
    doc.tag_mainbody_qry_count = tag_mainbody_qry_count;
    doc.tag_inlink_qry_count = tag_inlink_qry_count;

    doc.tag_title_count = freqs.fields_stats.tags_count["title"];
    if (doc.tag_title_count > 1) {
        // penalise docs with more than 1 `title` tag
        doc.tag_title_count = -doc.tag_title_count;
    }

    doc.tag_heading_count = freqs.fields_stats.tags_count["heading"];
    doc.tag_inlink_count = freqs.fields_stats.tags_count["inlink"];
    doc.tag_applet_count = freqs.fields_stats.tags_count["applet"];
    doc.tag_object_count = freqs.fields_stats.tags_count["object"];
    doc.tag_embed_count = freqs.fields_stats.tags_count["embed"];
  }

  void set_tag_qry_count(std::string field, size_t n) {
    switch (field_lookup.at(field)) {
    case F_STR_TITLE:
      tag_title_qry_count = n;
      break;

    case F_STR_HEADING:
      tag_heading_qry_count = n;
      break;

    case F_STR_MAINBODY:
      tag_mainbody_qry_count = n;
      break;

    case F_STR_INLINK:
      tag_inlink_qry_count = n;
      break;

    case MARKER_STOP_QRY_COUNT:
    default:
      break;
    }
  }
};

std::map<std::string, uint16_t> document_features::field_lookup = {
    {"title", F_STR_TITLE},       {"heading", F_STR_HEADING},
    {"mainbody", F_STR_MAINBODY}, {"inlink", F_STR_INLINK},
    {"applet", F_STR_APPLET},     {"object", F_STR_OBJECT},
    {"embed", F_STR_EMBED},
};
