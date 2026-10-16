import http from 'k6/http';
import { check, sleep } from 'k6';

export const options = {
  vus: 15,
  duration: '20s',
  thresholds: {
    http_req_failed: ['rate<0.01'],
    http_req_duration: ['p(95)<400'],
  },
};

export default function () {
  const r = http.get('http://localhost:8080/tasks');
  check(r, { 'list 200': (res) => res.status === 200 });
  sleep(0.3);
}
