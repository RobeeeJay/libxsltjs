{
  'targets': [
    {
      'target_name': 'nodexsltjs',
      'sources': [
        'src/libxsltjs.cc'
      ],
      'conditions': [
        ['OS=="win"', {
          # no Windows support yet...
        }, {
          'libraries': [
            '<!@(xml2-config --libs)',
            '<!@(xslt-config --libs)'
          ],
        }],
        ['OS=="mac"', {
          # cflags on OS X are stupid and have to be defined like this
          'xcode_settings': {
            'OTHER_CFLAGS': [
              '<!@(xml2-config --cflags)'
              '<!@(xslt-config --libs)'
            ]
          }
        }, {
          'cflags': [
            '<!@(xml2-config --cflags)',
            '<!@(xslt-config --libs)'
          ],
        }]
      ]
    }
  ]
}
