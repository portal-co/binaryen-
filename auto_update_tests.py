#!/usr/bin/env python

import os, sys, subprocess, difflib

print '[ processing and updating testcases... ]\n'

for asm in sorted(os.listdir('test')):
  if asm.endswith('.asm.js'):
    print '..', asm
    wasm = asm.replace('.asm.js', '.fromasm')
    actual, err = subprocess.Popen([os.path.join('bin', 'asm2wasm'), os.path.join('test', asm)], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    open(os.path.join('test', wasm), 'w').write(actual)

for wasm in sorted(os.listdir('test')):
  if wasm.endswith('.wast'):
    print '..', wasm
    asm = wasm.replace('.wast', '.2asm.js')
    actual, err = subprocess.Popen([os.path.join('bin', 'wasm2asm'), os.path.join('test', wasm)], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    open(os.path.join('test', asm), 'w').write(actual)

for t in sorted(os.listdir('test')):
  if t.endswith('.wast') and not t.startswith('spec'):
    print '..', t
    t = os.path.join('test', t)
    actual, err = subprocess.Popen([os.path.join('bin', 'binaryen-shell'), t, '--print-before'], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    actual = actual.replace('printing before:\n', '')

    open(t, 'w').write(actual)

for dot_s_dir in ['dot_s', 'llvm_autogenerated']:
  for s in sorted(os.listdir(os.path.join('test', dot_s_dir))):
    if not s.endswith('.s'): continue
    print '..', s
    wasm = s.replace('.s', '.wast')
    full = os.path.join('test', dot_s_dir, s)
    actual, err = subprocess.Popen([os.path.join('bin', 's2wasm'), full], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    assert err == '', 'bad err:' + err

    expected_file = os.path.join('test', dot_s_dir, wasm)
    open(expected_file, 'w').write(actual)

for wasm in ['address.wast']:#os.listdir(os.path.join('test', 'spec')):
  if wasm.endswith('.wast'):
    print '..', wasm
    asm = wasm.replace('.wast', '.2asm.js')
    proc = subprocess.Popen([os.path.join('bin', 'wasm2asm'), os.path.join('test', 'spec', wasm)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    actual, err = proc.communicate()
    assert proc.returncode == 0, err
    assert err == '', 'bad err:' + err
    expected_file = os.path.join('test', asm)
    open(expected_file, 'w').write(actual)

for t in sorted(os.listdir(os.path.join('test', 'passes'))):
  if t.endswith('.wast'):
    print '..', t
    passname = os.path.basename(t).replace('.wast', '')
    cmd = [os.path.join('bin', 'binaryen-shell'), '--print-after', ('--' + passname if passname != 'O' else '-O'), os.path.join('test', 'passes', t)]
    print '    ', ' '.join(cmd)
    actual, err = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    open(os.path.join('test', 'passes', passname + '.txt'), 'w').write(actual)

print '\n[ checking binary format testcases... ]\n'

for wast in sorted(os.listdir('test')):
  if wast.endswith('.wast') and not wast in []: # blacklist some known failures
    cmd = [os.path.join('bin', 'wasm-as'), os.path.join('test', wast), '-o', 'a.wasm']
    print ' '.join(cmd)
    if os.path.exists('a.wasm'): os.unlink('a.wasm')
    subprocess.check_call(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    assert os.path.exists('a.wasm')

    cmd = [os.path.join('bin', 'wasm-dis'), 'a.wasm', '-o', 'a.wast']
    print ' '.join(cmd)
    if os.path.exists('a.wast'): os.unlink('a.wast')
    subprocess.check_call(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    assert os.path.exists('a.wast')
    actual = open('a.wast').read()
    open(os.path.join('test', wast + '.fromBinary'), 'w').write(actual)

print '\n[ success! ]'

