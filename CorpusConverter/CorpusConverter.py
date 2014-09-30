import codecs
import os
import re
import sys
import getopt
import glob
import configparser
from lxml import etree

## Corpus Converter Version v1.0.1

config = configparser.ConfigParser()
special_tokens = None
special_token_max = 0

def GetPOS(linktag):
  fs = linktag.findall('.//' + linktag.tag[:-1] + 'f')
  for f in fs:
    if f.attrib.get('name') == 'msd':
      return f.attrib.get('value')
  return 'UNK'

# corpus 별 converter
def OANCConverter(filename, depth):
  global config, special_tokens, special_token_max
  if not filename.endswith('.txt'):
    return
  else:
    print('\t'*(depth+1) + filename[filename.rfind(os.sep)+1:])
  outfile = config['path']['OutputDirectory'] + os.sep + filename[len(config['path']['InputDirectory']):]
  MakeDirectory(outfile[:outfile.rfind(os.sep)])
  text = ''
  with codecs.open(filename, 'r', 'utf-8') as fin:
    text = fin.read()
  xmlfilename = filename[:-4] + '-hepple.xml'
  try:
    xml = etree.iterparse(xmlfilename, events=('end',))
    buff = []
    linebuff = []
    for event, elem in xml:
      if elem.tag[elem.tag.rfind('}')+1:] == 'graph':
        xmlprefix = elem.tag[:elem.tag.rfind('}')+1]
        regions = elem.findall('./' + xmlprefix + 'region')
        links = elem.findall('./' + xmlprefix + 'a')
        if len(regions) != len(links):
          print('xml parse error! :: ' + xmlfilename)
          return
        prev_end_idx = 0
        prev_begin_idx = 0
        for i in range(len(regions)):
          token = ''
          st_idx, ed_idx = (int(j) for j in regions[i].attrib.get('anchors').split())
          if special_tokens is not None and prev_end_idx != 0 and prev_end_idx == st_idx:
            if len(special_tokens) < special_token_max:
              special_tokens[text[prev_begin_idx:ed_idx]] = text[prev_begin_idx:prev_end_idx] + ' ' + text[st_idx:ed_idx]
          if '\n' in text[prev_end_idx:st_idx]:
            if linebuff:
              linebuff.append(config['separator']['SequnceEndSymbol'])
              buff.append(config['separator']['TokenSeparator'].join(linebuff))
              linebuff = []
            linebuff.append(config['separator']['SequnceBeginSymbol'])
          prev_end_idx = ed_idx
          prev_begin_idx = st_idx
          token = text[st_idx:ed_idx] + config['separator']['TagSeparator'] + GetPOS(links[i])
          linebuff.append(token)
        if linebuff:
          linebuff.append(config['separator']['SequnceEndSymbol'])
          buff.append(config['separator']['TokenSeparator'].join(linebuff))
    with codecs.open(outfile, 'w', 'utf-8') as fout:
      fout.write(config['separator']['LineSeparator'].join(buff))
  except FileNotFoundError:
    print('FileNotFoundError :: cannot find file dummy of "' + filename + '"')
    return
      
  #print('\t'*depth + filename[filename.rfind(os.sep)+1:])

def MakeDirectory(dirname):
  if dirname == '.':
    return True
  if not os.path.isdir(dirname):
    if dirname.rfind(os.sep) == -1:
      return False
    if MakeDirectory(dirname[:dirname.rfind(os.sep)]):
      try:
        os.mkdir(dirname)
      except FileExistsError:
        return True
      return True
    else:
      return False
  else:
    return True
  

def ScanDirectory(dirname, file_function, depth = 0):
  if not os.path.isdir(dirname):
    print("ERROR :: cannot find directory [" + dirname + "]")
    sys.exit(1)   
  if depth == 0:
    print("Start scan directory... ["+dirname+"]")
  else:
    print("\t"*depth + "sub directory : ["+os.path.split(dirname)[1]+"]")
  files = glob.glob(dirname + os.sep + '*')
  for file in files:
    if os.path.isdir(file):
      ScanDirectory(file, file_function, depth+1)
    elif os.path.isfile(file):
      file_function(file, depth)
    else:
      continue
  pass

def InitConfig(inifile):
  global config, special_tokens, special_token_max
  config.read(inifile)
  if str(config['info']['print']).lower() == 'on':
    print('======== Converter Info ========')
    for key in config['info']:
      if key == 'print':
        continue
      print(key + ' : ' + config['info'][key])
    print('corpus : ' + config['corpus']['name'])
    print('=================================')
  if str(config['option']['PrintSpecialTokenBoundary']).lower() == 'on':
    special_tokens = dict()
    special_token_max = int(config['option']['SpesialTokenBoundaryMaximum'])
    
                                              
    
def subfunc(matchobj):
  return os.sep

def main(args):
  global config, special_tokens
  inifile = './converter.ini'
  if args:
    inifile = args[0]
  InitConfig(inifile)
  indir = config['path']['InputDirectory']
  indir = re.sub(r'[/\\]+', subfunc, indir)
  if indir[-1] == os.sep:
    indir = indir[:-1]
    
  config['path']['OutputDirectory'] = re.sub(r'[/\\]+', subfunc, config['path']['OutputDirectory'])
  if config['path']['OutputDirectory'][-1] == os.sep:
    config['path']['OutputDirectory'] = config['path']['OutputDirectory'][:-1]

  tagsep = config['separator']['TagSeparator']
  tokensep = config['separator']['TokenSeparator']
  linesep = config['separator']['LineSeparator']

  tagsep = re.sub('\(space\)', ' ', tagsep)
  tagsep = re.sub('\(tab\)', '\t', tagsep)
  config['separator']['TagSeparator'] = re.sub('\(newline\)', '\n', tagsep)

  tokensep = re.sub('\(space\)', ' ', tokensep)
  tokensep = re.sub('\(tab\)', '\t', tokensep)
  config['separator']['TokenSeparator'] = re.sub('\(newline\)', '\n', tokensep)

  linesep = re.sub('\(space\)', ' ', linesep)
  linesep = re.sub('\(tab\)', '\t', linesep)
  config['separator']['LineSeparator'] = re.sub('\(newline\)', '\n', linesep)

  convertFunction = None
  if config['corpus']['name'] == 'OANC-GrAF':
    convertFunction = OANCConverter
  
  ScanDirectory(indir, convertFunction)

  with codecs.open('SpecialTokenBoundaryList.txt', 'w', 'utf-8') as fout:
    fout.write('\n'.join([i+'\t'+special_tokens[i] for i in special_tokens]))

if __name__=="__main__":
  main(sys.argv[1:])
