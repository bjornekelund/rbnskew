#!/usr/bin/env bash

set -euo pipefail

CREDENTIALS_FILE="../WEBCREDENTIALS"

dos2unix -q $CREDENTIALS_FILE

if [[ ! -f "$CREDENTIALS_FILE" ]]; then
    echo "ERROR: Credentials file '$CREDENTIALS_FILE' not found." >&2
    exit 1
fi

# Load credentials
# shellcheck disable=SC1090
source "$CREDENTIALS_FILE"

#printf 'HOST=<%q>\n' "$FTP_HOST"
#printf 'USER=<%q>\n' "$FTP_USER"
#printf 'DIR=<%q>\n' "$FTP_REMOTE_DIR"

: "${FTP_HOST:?FTP_HOST is not set in $CREDENTIALS_FILE}"
: "${FTP_USER:?FTP_USER is not set in $CREDENTIALS_FILE}"
: "${FTP_PASSWORD:?FTP_PASSWORD is not set in $CREDENTIALS_FILE}"

FTP_REMOTE_DIR="${FTP_REMOTE_DIR:-/}"

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 file1 file2 file3" >&2
    exit 1
fi

for file in "$@"; do
    if [[ ! -f "$file" ]]; then
        echo "ERROR: File not found: $file" >&2
        exit 1
    fi
done



for file in "$@"; do
    echo "Uploading $file..."

    url="ftp://$FTP_HOST${FTP_REMOTE_DIR%/}/$(basename "$file")"
#    printf 'URL=<%q>\n' "$url"

    curl \
        --fail \
        --silent \
        --show-error \
        --use-ascii \
        --user "$FTP_USER:$FTP_PASSWORD" \
        --upload-file "$file" \
        "$url"

#    echo "Uploaded $file"
done

echo "All files uploaded successfully."
