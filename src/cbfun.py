# cbgrep.py
## Python 3
# crunchbase

import cbutils
import datetime

def fmt(items):
    return ", ".join(
        ("%-50s" if index == 0 else "%-20s") % item
        for index, item in enumerate(items))

def main():
    # keys = cbutils.get_companies_keys()
    keys = (
        "name", "funding_total_usd", "founded_at",
        "last_funding_at", "funding_rounds", "category_list")
    print(fmt(keys))
    all_companies = cbutils.get_companies()
    companies = [
        company for company in all_companies if
            company["region"] == "SF Bay Area" and
            company["status"] == "operating" and
            cbutils.parse_date(company["last_funding_at"]).year >= 2015 and
            company["funding_rounds"] in "0 1" and
            2 < len(company["funding_total_usd"])]
    for company in companies:
        print(fmt(company[key] for key in keys))
    print(fmt(keys))
    print(len(companies))


if __name__ == "__main__":
    main()

