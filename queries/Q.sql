select * from tag where v glob 'Q*' and k not in ('brand:wikidata', 'name:etymology:wikidata') limit 100;
