# cbutils.py
## Crunchbase untils
import os
import csv
import datetime

SRCDIR = os.path.dirname(os.path.realpath(__file__))

def join(*paths):
    return os.path.abspath(os.path.join(*paths))

CBDIR = join(SRCDIR, "..", "raw", "crunchbase")

COMPANIES_PATH = join(CBDIR, "companies.csv")


def parse_date(s):
    return datetime.datetime.strptime(s, "%Y-%m-%d")


def get_companies():
    companies = []
    with open(COMPANIES_PATH) as csvfile:
        rows = tuple(csv.DictReader(csvfile))
    return rows


def get_companies_keys():
    with open(COMPANIES_PATH) as csvfile:
        return tuple(csv.DictReader(csvfile).fieldnames)

