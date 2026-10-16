import http from 'k6/http';
import { check, sleep } from 'k6';

export const options = {
  vus: 12,
  duration: '20s',
  thresholds: {
    http_req_failed: ['rate<0.01'],
    http_req_duration: ['p(95)<400'],
  },
};

export default function () {
  if (__ITER % 3 === 0) {
    const payload = JSON.stringify({
      title: `m-${__VU}-${__ITER}`,
      assignee: 'osipov',
      priority: 2,
    });
    const created = http.post('http://localhost:8080/tasks', payload, {
      headers: { 'Content-Type': 'application/json' },
    });
    check(created, { 'created 201': (r) => r.status === 201 });
  } else {
    const listed = http.get('http://localhost:8080/tasks');
    check(listed, { 'list 200': (r) => r.status === 200 });
  }
  sleep(0.4);
}
